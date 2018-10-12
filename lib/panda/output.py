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
        self.write('\n')

    def writeline(self, line):
        # Replace simple comments in .def files with doxygen-style back comments
        self.write((('  ' * self.indent) + line + '\n').replace('//', '///<'))

    def writelines(self, lines, line_end = ''):
        indented_lines = []
        for line in lines:
            indented_lines.append(('  ' * self.indent) + line)

        self.write((line_end + '\n').join(indented_lines) + '\n')

    def write_from_template(self, template_path, replacements):
        template = open(template_path)

        content = ''
        while True:
            line = template.readline()
            if not line:
                break

            matches = re.match('@IF\[(.+)\]@', line.strip())
            if matches:
                switch_name = matches.group(1)
                switch = replacements[switch_name]
                while True:
                    line = template.readline()
                    if not line or '@ENDIF@' in line:
                        break

                    if '@ELSE@' in line:
                        switch = not switch
                        continue

                    if switch:
                        content += line

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
                content += line
                indent = len(matches.group(1)) / 2
                name = matches.group(2)
                try:
                    content += self.custom_blocks[name].dump()
                    from_template = False
                except KeyError:
                    from_template = True

                while True:
                    line = template.readline()
                    if not line or '/* END CUSTOM */' in line:
                        break

                    if from_template:
                        content += line

                content += ('  ' * indent) + '/* END CUSTOM */\n'
                continue

            # otherwise, normal line
            content += line

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

        self.write(content)


class BufferOutput(Output):
    def __init__(self):
        Output.__init__(self)

        self.buffer = ''

    def write(self, text):
        self.buffer += text

    def dump(self, indent = 0):
        if not self.buffer:
            return ''

        if indent == 0:        
            return self.buffer
        else:
            content = ''
            lines = self.buffer.split('\n')
            for i in range(len(lines) - 1):
                if lines[i]:
                    content += ('  ' * indent) + lines[i] + '\n'
                else:
                    content += '\n'

            if lines[-1]:
                # last line did not end with \n
                content += ('  ' * indent) + lines[-1]

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

    def close(self):
        self._file.close()

    def write(self, text):
        self._file.write(text)

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
