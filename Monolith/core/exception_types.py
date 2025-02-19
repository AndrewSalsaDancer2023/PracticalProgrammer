class CollectionDeletionError(Exception):
    def __init__(self, msg="Unable to delete collection"):
        # Custom attributes
        self.msg = msg
        super().__init__(self.msg)  # Call the base class constructor


    def __str__(self):
        return self.msg
