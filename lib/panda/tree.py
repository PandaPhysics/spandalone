from base import Definition
from obj import Object
from physics import PhysicsObject
from inheritable import Inheritable

class Tree(Definition, Object, Inheritable):
    """
    Tree definition. Definition file syntax:

    {Name(>Parent)}
    <branch definitions>
    <function definitions>
    """

    _known_objects = []

    def __init__(self, line, source):
        Definition.__init__(self, line, '\\{([A-Z][a-zA-Z0-9]+)(>[A-Z][a-zA-Z0-9]+|)\\}$')
        Object.__init__(self, self.matches.group(1), source)
        Inheritable.__init__(self)

        Tree._known_objects.append(self)

        if self.matches.group(2):
            self.parent = self.matches.group(2)[1:]
        else:
            self.parent = 'TreeEntry'

    def make_inheritance_chain(self):
        Inheritable._make_inheritance_chain(self, Tree._known_objects)

        for objbranch in self.objbranches:
            objdef = next(o for o in PhysicsObject._known_objects if o.name == objbranch.objname)
            if objdef.is_singlet and (objbranch.conttype != '' or objbranch.init_size != ''):
                raise RuntimeError('Cannot create container of object ' + objdef.name)

    def write_header_includes(self, out):
        if self.parent == 'TreeEntry':
            out.writeline('#include "../../Framework/interface/TreeEntry.h"')
        else:
            out.writeline('#include "{parent}.h"'.format(parent = self.parent))

        included = []
        for objbranch in self.objbranches:
            if objbranch.objname not in included:
                out.writeline('#include "{brobj}.h"'.format(brobj = objbranch.objname))
                included.append(objbranch.objname)

        for include in self.includes:
            include.write(out)

    def write_initializers_copy(self, out):
        initializers = ['{parent}(_src)'.format(parent = self.parent)]
        for objbranch in self.objbranches:
            initializers.append(objbranch.cpyctor())
        for branch in self.branches:
            branch.init_copy(initializers, context = 'TreeEntry')

        out.writelines(initializers, ',')
