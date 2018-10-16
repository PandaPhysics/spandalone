./bin/generate.py [-o output_path] def_file

Def file can contain a namespace tag <namespace X>. If this tag is present, all objects
are created under namespace X.
Conversion between internal object representations and TTree branch names can be specified
by adding a code snippet in the def file. See PandaNano for an example.
