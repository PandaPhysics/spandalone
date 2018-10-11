from base import Definition

class OneLiner(Definition):
    """
    Single-liner C++ code to be directly written into target code.
    """

    def __init__(self, line, pattern):
        Definition.__init__(self, line, pattern)
        self.code = line.strip()

    def write(self, out):
        out.writeline(self.matches.group(0))


class Include(OneLiner):
    """
    C++ include lines.
    """

    def __init__(self, line):
        OneLiner.__init__(self, line, '#include +[^ ]+$')


class Typedef(OneLiner):
    """
    C++ typedef lines.
    """
    
    def __init__(self, line):
        OneLiner.__init__(self, line, 'typedef +[^ ]+ +[^ ]+$')

