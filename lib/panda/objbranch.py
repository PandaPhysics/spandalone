from base import Definition
from physics import PhysicsObject

class ObjBranch(Definition):
    """
    Composite object "branch" definition. Definition file syntax:
    <name>/<type>(|Collection|Array)
    where <type> is one of the PhysicsObjects.
    """

    def __init__(self, line):
        Definition.__init__(self, line, '([a-zA-Z_][a-zA-Z0-9_]*)/([^ \\(]+)(\\([0-9]+\\)|)( *//.+=.+)?$')

        self.name = self.matches.group(1)
        self.objname = self.matches.group(2)
        if self.matches.group(4):
            self.comment = self.matches.group(4)
            self.modifiers = dict([x.split('=') for x in self.matches.group(4).replace('//','').split()])
        else:
            self.comment = ''
            self.modifiers = {}
        if self.objname.endswith('Collection'):
            self.objname = self.objname.replace('Collection', '')
            self.conttype = 'Collection'
        elif self.objname.endswith('Array'):
            self.objname = self.objname.replace('Array', '')
            self.conttype = 'Array'
        else:
            self.conttype = ''

        self.init_size = self.matches.group(3).strip('()')

        try:
            # is this a valid object?
            objdef = PhysicsObject.get(self.objname)
        except KeyError:
            raise Definition.NoMatch()

        if objdef.is_singlet() and (self.conttype != '' or self.init_size != ''):
            raise RuntimeError('Cannot create container of object ' + objdef.name)

        if self.conttype == 'Array' and self.init_size == '':
            raise RuntimeError('Array object ' + objdef.name + ' needs a size')

    def type(self):
        return self.objname + self.conttype

    def write_decl(self, out):
        if self.conttype == 'Array':
            out.writeline('{objname}{type} {name} = {objname}{type}({size}, "{name}");'.format(objname = self.objname, type = self.conttype, name = self.name, size = self.init_size)
                          + self.comment)
        elif self.conttype == 'Collection' and self.init_size != '':
            out.writeline('{objname}{type} {name} = {objname}{type}("{name}", {size});'.format(objname = self.objname, type = self.conttype, name = self.name, size = self.init_size)
                          + self.comment)
        else:
            out.writeline('{objname}{type} {name} = {objname}{type}("{name}");'.format(objname = self.objname, type = self.conttype, name = self.name)
                          + self.comment)

    def write_set_status(self, out):
        out.writeline('{name}.setStatus(_tree, _branches.subList("{name}"));'.format(name = self.name))

    def write_get_status(self, out):
        out.writeline('blist += {name}.getStatus(_tree);'.format(name = self.name))

    def write_set_address(self, out):
        out.writeline('{name}.setAddress(_tree, _branches.subList("{name}"), _setStatus);'.format(name = self.name))

    def write_book(self, out):
        out.writeline('{name}.book(_tree, _branches.subList("{name}"));'.format(name = self.name))

    def write_release_tree(self, out):
        out.writeline('{name}.releaseTree(_tree);'.format(name = self.name))

    def cpyctor(self):
        return '{name}(_src.{name})'.format(name = self.name)

    def write_assign(self, out):
        out.writeline('{name} = _src.{name};'.format(name = self.name))

    def write_init(self, out):
        out.writeline('{name}.init();'.format(name = self.name))

    def write_dump(self, out):
        out.writeline('{name}.dump(_out);'.format(name = self.name))
