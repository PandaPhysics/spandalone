from base import Definition, integral_types

class Constant(Definition):
    """
    C++ const lines.
    """

    def __init__(self, line, source):
        Definition.__init__(self, line, '(?:static +|)(?:const +([^ ]+)|([^ ]+) +const) +([a-zA-Z0-9_]+(?:\[[^\]]+\])*)([^\(\)]+)$')
        self.type = self.matches.group(1)
        if self.type is None:
            self.type = self.matches.group(2)

        self.decl = self.matches.group(3)
        self.value = self.matches.group(4)

        # if value ends with semicolon in one line, either the value must be given in the .cc file or is given by a one-liner init expression

        if not self.value.endswith(';'):
            # we are in the middle of a multi-line brace-init expression
            self.value += '\n'
    
            depth = self.value.count('{') - self.value.count('}')
    
            while True:
                line = source.readline()
                if line == '':
                    break
    
                self.value += line
    
                depth += line.count('{')
                depth -= line.count('}')
    
                if depth == 0:
                    break
    
            self.value = self.value[:-1] # last character is \n
    
            if not self.value.endswith(';'):
                raise RuntimeError('Const expression %s %s did not end with a semicolon' % (self.type, self.decl))

        # remove the trailing semicolon
        self.value = self.value[:-1]

    def write_decl(self, out, context):
        if context == 'global':
            out.writeline('{type} const {decl}{value};'.format(type = self.type, decl = self.decl, value = self.value))
        else:
            if self.type in integral_types and '[' not in self.decl:
                out.writeline('static {type} const {decl}{value};'.format(type = self.type, decl = self.decl, value = self.value))
            else:
                out.writeline('static {type} const {decl};'.format(type = self.type, decl = self.decl))

    def write_def(self, out, cls):
        # called only by PhysicsObject to define the values of class static const in the global scope
        if self.type not in integral_types or '[' in self.decl:
            out.writeline('/*static*/')
            out.writeline('{type} const {cls}::{decl}{value};'.format(type = self.type, cls = cls, decl = self.decl, value = self.value))


class Assert(Definition):
    """
    Compile-time assertions.
    Takes any C++ expression that evaluates to a boolean with syntax
    ASSERT <expr>
    """

    @staticmethod
    def write(assertions, out):
        # write the assertions as an enum of values sizeof(char[N])
        # if the condition is false, N is negative -> compile error.
        asserts = []
        for ia, assertion in enumerate(assertions):
            asserts.append('AST{i} = sizeof(char[({expr}) ? 1 : -1])'.format(i = ia, expr = assertion.matches.group(1)))

        if len(asserts) != 0:
            enum = Enum('enum Assertions {', asserts)
            enum.write_decl(out, context = 'global')

    def __init__(self, line):
        Definition.__init__(self, line, 'ASSERT +(.+)')


class Enum(Definition):
    """
    C++ enum definition.
    """

    def __init__(self, line, source):
        Definition.__init__(self, line, 'enum *([^ ]+) *\\{')

        self.name = self.matches.group(1)

        if type(source) is list:
            # a list is already given
            self.elements = source

        else:
            # otherwise parse the source text
            self.elements = []
    
            while True:
                line = source.readline()
                if line == '':
                    break
    
                if line.strip().startswith('}'):
                    break
    
                for elem in line.strip().split(','):
                    elem = elem.strip()
                    if elem:
                        self.elements.append(elem)

        # last entry is always n{name}s (e.g. enum Trigger -> nTriggers)
        last_name = 'n' + self.name
        if last_name.endswith('y'):
            last_name = last_name[:-1] + 'ies'
        elif last_name.endswith('s') or last_name.endswith('x') or last_name.endswith('z'):
            last_name += 'es'
        else:
            last_name += 's'

        self.elements.append(last_name)

    def write_decl(self, out, context):
        out.writeline('enum ' + self.name + ' {')
        out.indent += 1
        out.writelines(self.elements, ',')
        out.indent -= 1
        out.writeline('};')

        out.newline()

        if context == 'global':
            out.writeline('extern TString {name}Name[{size}];'.format(name = self.name, size = self.elements[-1]))
        elif context == 'class':
            out.writeline('static TString {name}Name[{size}];'.format(name = self.name, size = self.elements[-1]))

    def write_def(self, out, cls = ''):
        enum_names = []
        for elem in self.elements[:-1]:
            enum_names.append('"{elem}"'.format(elem = elem))

        if cls == '':
            out.writeline('TString {name}Name[] = {{'.format(name = self.name))
        else:
            out.writeline('TString {cls}::{name}Name[] = {{'.format(cls = cls, name = self.name))
        out.indent += 1
        out.writelines(enum_names, ',')
        out.indent -= 1
        out.writeline('};')

