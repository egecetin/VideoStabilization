#!/usr/bin/python3

import errno
import os
import sys
from optparse import OptionParser


if __name__ == "__main__":
    # Parse inputs
    parser = OptionParser()
    parser.usage = "Dump symbol files using Google breakpad dump_syms"
    parser.add_option("", "--dump-syms")
    parser.add_option("", "--binary-dir")
    parser.add_option("", "--output-dir")
    (options, args) = parser.parse_args()

    dumpSymsExe = options.dump_syms
    binaryDir = options.binary_dir
    outputDir = options.output_dir

    # Check all inputs
    if dumpSymsExe is None or binaryDir is None or outputDir is None:
        print("Provide all arguments")
        sys.exit(1)

    if os.path.isfile(binaryDir):
        binaryPaths = [binaryDir]
    else:
        binaryPaths = [
            os.path.join(binaryDir, filePath)
            for filePath in os.listdir(binaryDir)
            if os.path.isfile(os.path.join(binaryDir, filePath))
            and not os.path.islink(os.path.join(binaryDir, filePath))
        ]

    for binaryPath in binaryPaths:
        # Prepare command to dump symbols
        outFilePath = outputDir + os.path.basename(binaryPath) + ".sym"
        command = f"{dumpSymsExe} {binaryPath} > {outFilePath}"

        os.system(command)

        # Prepare desired folder structure
        symFile = open(outFilePath, "r")
        firstLine = symFile.readline().split(" ")

        symbolsDir = outputDir + "/" + firstLine[-1].strip() + "/" + firstLine[-2] + "/"

        try:
            os.makedirs(symbolsDir)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise
        os.rename(outFilePath, symbolsDir + firstLine[-1].strip() + ".sym")
        os.system(f"strip {binaryPath}")
