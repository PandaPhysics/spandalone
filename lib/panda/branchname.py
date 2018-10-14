from panda.base import Definition

class BranchName(Definition):
    def __init__(self, source):
        self.parse = []
        self.generate = []

        reading = ''
        while True:
            line = source.readline()

            if line.strip().startswith('<end'):
                break

            if '(parse)' in line:
                reading = 'parse'
                continue

            if '(generate)' in line:
                reading = 'generate'
                continue

            if reading == 'parse':
                self.parse.append(line.rstrip('\n'))
            elif reading == 'generate':
                self.generate.append(line.rstrip('\n'))

    def write_parse(self, out):
        for line in self.parse:
            out.writeline(line)

    def write_generate(self, out):
        for line in self.generate:
            out.writeline(line)
