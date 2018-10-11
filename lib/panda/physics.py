from base import Definition
from obj import Object
from branch import Branch
from output import FileOutput

class PhysicsObject(Definition, Object):
    """
    Physics object definition. Definition file syntax:
    
    [Name(>Parent)(:SINGLE)]
    <variable definitions>
    <function definitions>
    """

    _known_objects = []

    @staticmethod
    def get(name):
        try:
            return next(o for o in PhysicsObject._known_objects if o.name == name)
        except StopIteration:
            raise KeyError('PhysicsObject.get({name})'.format(name = name))

    def __init__(self, line, source):
        """
        Argument: re match object
        """

        Definition.__init__(self, line, '\\[([A-Z][a-zA-Z0-9]+)(>[A-Z][a-zA-Z0-9]+|)(\\:SINGLE|)\\] *$')
        PhysicsObject._known_objects.append(self)

        # is this a singlet?
        if self.matches.group(3) == ':SINGLE':
            self.parent = 'Singlet'
            self._singlet = True
        else:
            self.parent = 'Element'
            self._singlet = False

        # if >parent is present, update the parent class name
        if self.matches.group(2):
            self.parent = self.matches.group(2)[1:]
            self._singlet = None

        Object.__init__(self, self.matches.group(1), source)

        if len([f for f in self.functions if f.is_pure_virtual]) == 0:
            self.instantiable = True
        else:
            self.instantiable = False

    def is_singlet(self):
        if self._singlet is None:
            return PhysicsObject.get(self.parent).is_singlet()
        else:
            return self._singlet

    def inheritance(self):
        inheritance = [self]
        while True:
            try:
                inheritance.insert(0, PhysicsObject.get(inheritance[0].parent))
            except KeyError:
                break

        return inheritance

    def write_header_includes(self, out):
        if self.parent == 'Element':
            out.writeline('#include "../../Framework/interface/Element.h"')
        elif self.parent == 'Singlet':
            out.writeline('#include "../../Framework/interface/Singlet.h"')
        else:
            out.writeline('#include "{parent}.h"'.format(parent = self.parent))

        for include in self.includes:
            out.writeline(include.code)

        included = []
        for branch in self.branches:
            # includes for ref branches
            if hasattr(branch, 'refname') and branch.objname not in included:
                included.append(branch.objname)
                stmt = '#include "{obj}.h"'.format(obj = branch.objname)
                out.writeline(stmt)

    def write_datastore_inherited_members(self, out):
        inheritance = self.inheritance()

        for ancestor in inheritance:
            if ancestor is self:
                break

            out.writeline('/* {name}'.format(name = ancestor.name))

            for branch in ancestor.branches:
                branch.write_decl(out, context = 'datastore')

            out.writeline('*/')

    def write_datastore_members(self, out):
        for branch in self.branches:
            branch.write_decl(out, context = 'datastore')

    def write_inherited_public_members(self, out):
        inheritance = self.inheritance()

        if self.is_singlet():
            context = 'Singlet'
        else:
            context = 'Element'

        for ancestor in inheritance:
            if ancestor is self:
                break

            out.writeline('/* {name}'.format(name = ancestor.name))

            for branch in ancestor.branches:
                if not hasattr(branch, 'refname') and branch.name.endswith('_'):
                    continue

                branch.write_decl(out, context = context)

            out.writeline('*/')

    def write_public_members(self, out):
        if self.is_singlet():
            context = 'Singlet'
        else:
            context = 'Element'

        for branch in self.branches:
            if not hasattr(branch, 'refname') and branch.name.endswith('_'):
                continue

            branch.write_decl(out, context = context)

    def write_inherited_protected_members(self, out):
        inheritance = self.inheritance()

        if self.is_singlet():
            context = 'Singlet'
        else:
            context = 'Element'

        for ancestor in inheritance:
            if ancestor is self:
                break

            out.writeline('/* {name}'.format(name = ancestor.name))

            for branch in ancestor.branches:
                if hasattr(branch, 'refname') or not branch.name.endswith('_'):
                    continue

                branch.write_decl(out, context = context)

            out.writeline('*/')

    def write_protected_members(self, out):
        if self.is_singlet():
            context = 'Singlet'
        else:
            context = 'Element'

        for branch in self.branches:
            if hasattr(branch, 'refname') or not branch.name.endswith('_'):
                continue

            branch.write_decl(out, context = context)

    def write_initializers_default(self, out, element_private = False):
        if not self.instantiable:
            return

        if self.is_singlet():
            context = 'Singlet'
            initializers = ['{parent}(_name)'.format(parent = self.parent)]
        else:
            context = 'Element'
            if element_private:
                initializers = ['{parent}(_array)'.format(parent = self.parent)]
            else:
                initializers = ['{parent}(new {name}Array(1, _name))'.format(parent = self.parent, name = self.name)]

        for branch in self.branches:
            branch.init_default(initializers, context = context)

        out.writelines(initializers, ',')

    def write_initializers_copy(self, out):
        if not self.instantiable:
            return

        if self.is_singlet():
            context = 'Singlet'
            initializers = ['{parent}(_src.name_)'.format(parent = self.parent)]
        else:
            context = 'Element'
            initializers = ['{parent}(new {name}Array(1, _src.getName()))'.format(parent = self.parent, name = self.name)]

        for branch in self.branches:
            branch.init_copy(initializers, context = context)

        out.writelines(initializers, ',')

    def write_initializers_element(self, out):
        if self.is_singlet():
            return

        initializers = ['{parent}(_data, _idx)'.format(parent = self.parent)]
        for branch in self.branches:
            branch.init_standard(initializers, context = 'Element')
        out.writelines(initializers, ',')

    def write_destructor(self, out):
        if self.parent != 'Element':
            out.writeline('if (_recursive)')
            out.indent += 1
            out.writeline('{parent}::destructor(kTRUE);'.format(parent = self.parent))
            out.indent -= 1
