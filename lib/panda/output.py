import os
import re

class Output(object):
    """
    Base class for output.
    """

    def __init__(self):
        self.custom_blocks = {}
        self.indent = 0

    def write(self, text):
        raise NotImplementedError('Output.write')

    def dump(self, indent = 0):
        raise NotImplementedError('Output.dump')

    def newline(self):
        raise NotImplementedError('Output.newline')

    def writeline(self, line):
        # Replace simple comments in .def files with doxygen-style back comments
        self.write(line.replace('//', '///<'))
        self.newline()

    def writelines(self, lines, line_end = ''):
        for line in lines[:-1]:
            self.writeline(line + line_end)

        if len(lines) != 0:
            self.writeline(lines[-1])

    def write_from_template(self, template_path, replacements):
        with open(template_path) as source:
            lines = source.read().split('\n')

        content = self._generate_from_template(lines, replacements)
        self.write(content)

    def _generate_from_template(self, lines, replacements):
        content = ''
        iline = 0
        while iline != len(lines):
            line = lines[iline]
            iline += 1
            if not line:
                content += '\n'
                continue

            matches = re.match('@IF\[(.+)\]@', line.strip())
            if matches:
                block = []
                switch_name = matches.group(1)
                switch = replacements[switch_name]
                while True:
                    line = lines[iline]
                    iline += 1
                    if '@ENDIF@' in line:
                        break

                    if '@ELSE@' in line:
                        switch = not switch
                        continue

                    if switch:
                        block.append(line)

                content += self._generate_from_template(block, replacements)

                continue

            matches = re.match('( *)@(.+)@$', line.rstrip())
            if matches:
                # replacement-only line; if replacing with an Output, make it indentation-aware.
                indent = len(matches.group(1)) / 2
                key = matches.group(2)
                value = replacements[key]
                
                if isinstance(value, Output):
                    content += value.dump(indent)
                else:
                    content += ('  ' * indent) + value + '\n'

                continue

            matches = re.match('( *)\/\* BEGIN CUSTOM (.+) \*\/', line)
            if matches:
                content += line + '\n'
                indent = len(matches.group(1)) / 2
                name = matches.group(2)
                try:
                    content += self.custom_blocks[name].dump()
                    from_template = False
                except KeyError:
                    from_template = True

                while True:
                    line = lines[iline]
                    iline += 1
                    if '/* END CUSTOM */' in line:
                        break

                    if from_template:
                        content += line + '\n'

                content += ('  ' * indent) + '/* END CUSTOM */\n'
                continue

            # otherwise, normal line
            content += line + '\n'

        for key, value in replacements.iteritems():
            if isinstance(value, Output):
                # type BufferOutput
                value = value.dump()
            elif type(value) is not str:
                continue

            content = content.replace('@%s@' % key, value)

        # remove trailing spaces from each line
        content = re.sub(r'[ \t]+$', '', content, flags = re.M)
        # remove >double newlines
        content = re.sub(r'\n\n+', '\n\n', content)

        return content


class BufferOutput(Output):
    def __init__(self):
        Output.__init__(self)

        self.buffer = []

    def write(self, text):
        if len(self.buffer) == 0:
            self.buffer.append((-1, ''))

        idnt, txt = self.buffer[-1]
        if idnt == -1:
            idnt = self.indent

        self.buffer[-1] = (idnt, txt + text)

    def newline(self):
        self.buffer.append((-1, ''))

    def dump(self, indent = 0):
        if len(self.buffer) == 0:
            return ''

        content = ''
        for idnt, line in self.buffer:
            if len(content) != 0:
                content += '\n'

            if len(line) == 0:
                continue

            if idnt + indent >= 0:
                content += ('  ' * (idnt + indent))
            content += line

        return content


class FileOutput(Output):
    """
    Helper tool to write C++ code output.
    """

    def __init__(self, fname, preserve_custom = True):
        Output.__init__(self)

        if preserve_custom:
            try:
                original = open(fname)
            except IOError:
                pass
            else:
                while True:
                    line = original.readline()
                    if not line:
                        break
                  
                    matches = re.match('/\* BEGIN CUSTOM (.+) \*/', line.strip())
                    if matches is None:
                        continue
    
                    block = BufferOutput()
                    while True:
                        line = original.readline()
                        if not line or '/* END CUSTOM */' in line:
                            break

                        block.write(line)
    
                    self.custom_blocks[matches.group(1)] = block
                
                original.close()

        self._fname = fname

        directory = os.path.dirname(fname)
        if not os.path.isdir(directory):
            os.makedirs(directory)

        self._file = open(fname, 'w')

        self._linehead = True

    def close(self):
        self._file.close()

    def write(self, text):
        if self._linehead and text != '\n':
            self._file.write('  ' * self.indent)

        self._file.write(text)
        self._linehead = text.endswith('\n')

    def newline(self):
        self.write('\n')

    def dump(self, indent = 0):
        self._file.flush()
        with open(self._fname) as f:
            if indent == 0:
                return f.read()
            else:
                content = ''
                for line in f:
                    if line.strip() != '':
                        content += ('  ' * indent) + line
                    else:
                        content += line

                return content
