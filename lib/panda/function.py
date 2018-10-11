import re

from base import Definition

class Function(Definition):
    """
    Function branch definition. Write as a C++ function within the given scope.
    """

    def __init__(self, line, source):
        Definition.__init__(self, line, '((?:virtual +|static +|)([^\(]+) +([^ \(]+\([^\)]*\)(?: +const|))(?: +override|))(.*)')

        self.decl = self.matches.group(1) # includes virtual/static, const, override
        self.type = self.matches.group(2) # return type
        self.signature = self.matches.group(3) # function name and arguments

        self.impl = self.matches.group(4).strip()

        if re.match('^ *= *0 *;$', self.impl):
            self.is_pure_virtual = True
        else:
            self.is_pure_virtual = False

        if self.impl.endswith(';'):
            # implementation must be given by hand in the .cc file
            return

        depth = self.impl.count('{') - self.impl.count('}')

        if self.impl != '' and depth == 0:
            # a one-liner
            return

        while True:
            line = source.readline()
            if line == '':
                break

            self.impl += line

            depth += line.count('{')
            depth -= line.count('}')

            if depth == 0:
                break

    def write_decl(self, out, context):
        if context == 'global':
            out.writeline(self.decl + ';')

        elif context == 'class':
            if self.impl != ';' and '\n' not in self.impl: # a one-liner -> write directly in decl
                out.writeline('{decl} {impl}'.format(decl = self.decl, impl = self.impl))
            else:
                out.writeline('{decl};'.format(decl = self.decl))

    def write_def(self, out, cls = ''):
        # comment out default arguments - won't work if the default value is a string that contains , or )
        signature = re.sub(' *= *[^,)]+', lambda m: '/*' + m.group(0) + '*/', self.signature)
        if cls:
            if '\n' not in self.impl:
                # was a one-liner
                return

            # context is the class name
            out.writeline(self.type)
            out.writeline('{cls}::{sign}'.format(cls = cls, sign = signature))
            for line in self.impl.split('\n'):
                out.writeline(line)

        else:
            out.writeline(self.type)
            out.writeline('{sign}'.format(sign = signature))
            for line in self.impl.split('\n'):
                out.writeline(line)
