from base import Definition
from physics import PhysicsObject

class Reference(Definition):
    """
    Sets reference within a tree definition. Syntax:
    <branch>(.<branch>)-><collection>
    """

    def __init__(self, line):
        Definition.__init__(self, line, '([^ ]+)->([^ ]+)')
        self.ref_name = self.matches.group(1).split('.')
        self.target = self.matches.group(2)

    def write_def(self, out, objbranches):
        """
        Part of the tree entry constructor code to pass the target collection pointer to the reference.
        """

        if len(self.ref_name) == 1:
            out.writeline('{rname}Container_ = &{target};'.format(rname = '.'.join(self.ref_name), target = self.target))
        else:
            bname = self.ref_name[-2]
            branch = next(b for b in objbranches if b.name == bname)
            objdef = PhysicsObject.get(branch.objname)

            if objdef.is_singlet():
                out.writeline('{rname}Container_ = &{target};'.format(rname = '.'.join(self.ref_name), target = self.target))
            else:
                out.writeline('{rname}.data.{bname}Container_ = &{target};'.format(rname = '.'.join(self.ref_name[:-1]), bname = self.ref_name[-1], target = self.target))
