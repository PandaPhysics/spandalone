import re

class Definition(object):
    """
    Base class for all objects reading from the definitions file.
    """

    class NoMatch(Exception):
        pass

    def __init__(self, line, pattern):
        matches = re.match(pattern, line)
        if matches is None:
            raise Definition.NoMatch()

        self.matches = matches


integral_types = [
    'char',
    'unsigned char',
    'short',
    'unsigned short',
    'int',
    'unsigned int',
    'unsigned',
    'long',
    'unsigned long',
    'long long',
    'unsigned long long',
    'Char_t',
    'UChar_t',
    'Short_t',
    'UShort_t',
    'Int_t',
    'UInt_t',
    'Long64_t',
    'ULong64_t'
]
