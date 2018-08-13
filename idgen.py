#!/usr/bin/env python3
"""Utility class and app for generating constants for machine classes"""

from csv import DictReader
from collections import defaultdict
from io import StringIO

class MappingGen():
    """Generate QSTR/ROM_INT's and enums for device and pin indexes"""

    def __init__(self, prefix="", cmods=None):
        self.prefix = prefix
        self.cmods = cmods if cmods is not None else dict()
        self.qstr = defaultdict(dict)
        self.enum = defaultdict(dict)

    def read(self, dfile):
        """Read CSV data and populate entries for mapping indexes"""

        reader = DictReader(filter(lambda row: row[0] != '#', dfile),
                            ("class", "id", "cname", "mpname", "alias"),
                            skipinitialspace=True)

        for row in reader:
            if row["mpname"] is not None:
                self.qstr[row["class"]][row["mpname"]] = row["id"]
                if row["alias"] is not None:
                    self.qstr[row["class"]][row["alias"]] = row["id"]
            self.enum[row["class"]][row["cname"]] = row["id"]

    def generate(self):
        """Generate strings for QSTR and enums and return as tuple"""

        macros = StringIO()
        for cls, vals in self.qstr.items():
            macros.write("#define MACHINE_{}_IDS \\\n".format(cls.upper()))
            for name, idv in sorted(vals.items(), key=lambda x: int(x[1])):
                macros.write("    {{ MP_ROM_QSTR(MP_QSTR_{}), "
                             "MP_ROM_INT({}) }}, \\\n".format(name, idv))

            macros.write("    // no more\n\n")

        enums = StringIO()
        prefix = "" if self.prefix == "" else self.prefix + "_"
        for cls, vals in self.enum.items():
            cmod = self.cmods.get(cls, cls)
            enums.write("typedef enum {}{}Name {{\n".format(prefix, cmod))
            for name, idv in sorted(vals.items(), key=lambda x: int(x[1])):
                enums.write("    {}{} = {},\n".format(prefix, name, idv))

            enums.write("    {}{}COUNT = {}\n".format(prefix, cmod, len(vals)))
            enums.write("}} {}{}Name;\n\n".format(prefix, cmod))

        return (macros.getvalue(), enums.getvalue())


def main():
    """Utility for mapping CSV file to enum.h and qstr.h"""

    import argparse
    from os.path import basename, splitext

    ap = argparse.ArgumentParser(description="Generate ID mappings machine classes")
    ap.add_argument("-p", "--prefix", default="",
                    help="optional additional prefix to add to all C identifiers")
    ap.add_argument("-o", "--output", help="output file basename (instead of input)")
    ap.add_argument("input_csv", help="file with mappings")

    args = ap.parse_args()

    mpg_gen = MappingGen(args.prefix, {"Pin": "GPIO", "NVSBdev": "NVS",
                                       "SD": "SDFatFS", "SDSPI": "SD",
                                       "SDCARD": "SD"})

    with open(args.input_csv, "r", newline="") as mpg_file:
        mpg_gen.read(mpg_file)

    base_output = args.output if args.output else splitext(basename(args.input_csv))[0]

    with open(base_output + "_qstr.h", "w") as qstr_file:
        with open(base_output + "_enum.h", "w") as enum_file:
            qstr, enum = mpg_gen.generate()

            qstr_file.write(qstr)
            enum_file.write(enum)


if __name__ == "__main__":
    main()
