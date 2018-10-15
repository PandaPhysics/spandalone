class Inheritable(object):
    """
    Mixin for objects with a parent.
    """

    def __init__(self):
        self.parent = ''
        self.inheritance = []

    def _make_inheritance_chain(self, obj_list):
        self.inheritance = [self]
        while True:
            try:
                parent_obj = next(o for o in obj_list if o.name == self.inheritance[0].parent)
            except StopIteration:
                break

            self.inheritance.insert(0, parent_obj)

        self.inheritance.pop() # drop self
