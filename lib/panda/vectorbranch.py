from panda.branch import Branch

class VectorBranch(Branch):
    """
    Branch stored as a std::vector instead of a C array for collections.
    """

    def write_decl(self, out, context):
        if context == 'datastore':
            if self.is_array():
                raise RuntimeError('VectorBranch cannot have an array dimension and be a member of a collection element.')
            else:
                template = 'std::vector<{type}>* {name}{{0}};'
        elif context == 'Singlet' or context == 'TreeEntry':
            if 'm' in self.modifier:
                template = 'mutable {type} {name}{arrdef}{{{init}}};'
            else:
                template = '{type} {name}{arrdef}{{{init}}};'
        elif context == 'Element':
            if self.is_array():
                raise RuntimeError('VectorBranch cannot have an array dimension and be a member of a collection element.')
            else:
                template = '{type}& {name};'

        line = template.format(type = self.typename(), name = self.name, arrdef = self.arrdef_text(), init = self.init)

        if '!' in self.modifier:
            line += ' // transient'

        if context != 'datastore':
            line += self.comment

        out.writeline(line)

    def write_allocate(self, out, context):
        # context must be datastore
        out.writeline('{name} = new std::vector<{type}>(nmax_);'.format(name = self.name, type = self.typename()))

    def write_deallocate(self, out, context):
        # context must be datastore
        out.writeline('delete {name};'.format(name = self.name))
        out.writeline('{name} = 0;'.format(name = self.name))

    def write_set_address(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = '_name'
        elif context == 'TreeEntry':
            namevar = '""'

        if self.is_array():
            ptr = self.name
        else:
            ptr = '&' + self.name

        out.writeline('panda::utils::setAddress(_tree, {namevar}, "{name}", {ptr}, _branches, _setStatus);'.format(namevar = namevar, name = self.name, ptr = ptr))

    def write_book(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            out.writeline('panda::utils::book(_tree, _name, "{name}", "std::vector<{type}>", &{name}, _branches);'.format(name = self.name, type = self.typename()))

        else:
            Branch.write_book(self, out, context)

    def write_resize_vectors(self, out, context):
        pass

