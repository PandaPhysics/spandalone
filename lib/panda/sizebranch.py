from panda.base import Definition

class SizeBranch(Definition):
    def __init__(self, source):
        self.parse = ''
        self.generate = ''

        reading = ''
        while True:
            line = source.readline()

            if '<end sizebranch>' in line:
                break

            if '(parse)' in line:
                reading = 'parse'
                continue

            if '(generate)' in line:
                reading = 'generate'
                continue

            if reading == 'parse':
                self.parse += line
            elif reading == 'generate':
                self.generate += line

    def write_parse(self, out):
        out.write(self.parse)

    def write_generate(self, out):
        out.write(self.generate)
