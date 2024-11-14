# PortAudio Repository Whitespace Linter
#
# Run this script from the root of the repository using the command:
#   python pa_whitelint.py
#
# Check all source files for the following:
#   1. Consistent line endings are used throughout each file.
#   2. No tabs are present. Use spaces for indenting.
#   3. Indenting: leading whitespace is usually a multiple of 4 spaces,
#      with permissive exceptions for continuation lines.
#   4. Lines have no trailing whitespace.
#   5. No non-ASCII or weird control characters are present.
#   6. End-of-line is present at end-of-file.
#   7. No empty (or whitespace) lines at end-of-file.


from pathlib import Path
import re
import sys

# Configuration:

# Check these file types:
sourceFileTypes = ["*.c", "*.h", "*.cpp", "*.cxx", "*.hxx"]

# Scan these directories
dirs = ["src", "include", "examples", "test", "qa"]

# Exclude files or directories with the following names:
excludePathParts = [
    "ASIOSDK",
    "iasiothiscallresolver.cpp",
    "iasiothiscallresolver.h",
    "mingw-include",
]

indentSpaceCount = 4

verbose = True
checkBadIndenting = True
verboseBadIndenting = True

# (End configuration)


class FileStatus:
    """Issue status for a particular file. Stores issue counts for each type of issue."""

    def __init__(self, path):
        self.path = path
        issueNames = [
            "has-inconsistent-line-endings",
            "has-tabs",
            "has-bad-indenting",
            "has-trailing-whitespace",
            "has-bad-character",
            "has-empty-line-at-end-of-file",
            "has-no-eol-character-at-end-of-file",
        ]
        self.issueCounts = dict.fromkeys(issueNames, 0)

    def incrementIssueCount(self, issueName):
        assert issueName in self.issueCounts # catch typos in issueName
        self.issueCounts[issueName] += 1

    def hasIssue(self, issueName):
        return self.issueCounts[issueName] > 0

    def hasIssues(self):
        return any(count > 0 for count in self.issueCounts.values())

    def issueSummaryString(self):
        return str.join(", ", [name for name in self.issueCounts if self.issueCounts[name] > 0])


def multilineCommentIsOpenAtEol(lineText, wasOpenAtStartOfLine):
    isOpen = wasOpenAtStartOfLine
    index = 0
    end = len(lineText)
    while index != -1 and index < end:
        if isOpen:
            index = lineText.find(b"*/", index)
            if index != -1:
                isOpen = False
                index += 2
        else:
            index = lineText.find(b"/*", index)
            if index != -1:
                isOpen = True
                index += 2
    return isOpen


def allowStrangeIndentOnFollowingLine(lineText):
    """Compute whether a non-standard indent is allowed on the following line.
    A line allows an unusual indent to follow if it is the beginning of a
    multi-line function parameter list, an element of a function parameter list,
    or an incomplete expression (binary operator, etc.).
    """
    s = lineText.strip(b" ")
    if len(s) == 0:
        return False
    if s.rfind(b"*/") == (len(s) - 2):  # line has a trailing comment, strip it
        commentStart = s.rfind(b"/*")
        if commentStart != -1:
            s = s[:commentStart].strip(b" ")
            if len(s) == 0:
                return False

        if len(s) == 0:
            return False

    okChars = b'(,\\+-/*=&|?:"'
    if s[-1] in okChars: # non-comment program text has trailing okChar: '(' or ',' etc.
        return True
    return False


def allowStrangeIndentOfLine(lineText):
    """Compute whether a non-standard indent is allowed on the line.
    A line is allowed an unusual indent if it is the continuation of an
    incomplete expression (binary operator, etc.).
    """
    s = lineText.strip(b" ")
    if len(s) == 0:
        return False

    okChars = b'+-/*=&|?:)"'
    if s[0] in okChars:
        return True
    return False


# Run the checks over all files specified by [sourceFileTypes, dirs, excludePathParts]:

statusSummary = []
for dir in dirs:
    for ext in sourceFileTypes:
        for path in Path(dir).rglob(ext):
            if any(part in path.parts for part in excludePathParts):
                continue

            # during development, uncomment the following 2 lines and select a specific path:
            #if not "qa" in path.parts:
            #    continue

            data = path.read_bytes()

            status = FileStatus(path)
            statusSummary.append(status)

            # Perform checks:

            # 1. Consistent line endings
            # check and then normalize to \n line endings for the benefit of the rest of the program
            if b"\r" in data and b"\n" in data:
                # CRLF (Windows) case: check for stray CR or LF, then convert CRLF to LF
                assert not b"\f" in data  # we'll use \f as a sentinel during conversion
                d = data.replace(b"\r\n", b"\f")
                if b"\r" in d:
                    status.incrementIssueCount("has-inconsistent-line-endings")
                    if verbose:
                        print("error: {0} stray carriage return".format(path))
                if b"\n" in d:
                    status.incrementIssueCount("has-inconsistent-line-endings")
                    if verbose:
                        print("error: {0} stray newline".format(path))
                data = d.replace(b"\f", b"\n")  # normalize line endings
            elif b"\r" in data:
                # CR (Classic Mac) case: convert CR to LF
                data = d.replace(b"\r", b"\n")  # normalize line endings
            else:
                # LF (Unix) case: no change
                pass

            lines = data.split(b"\n")  # relies on newline normalization above

            # 2. Absence of tabs
            lineNo = 1
            for line in lines:
                if b"\t" in line:
                    status.incrementIssueCount("has-tabs")
                    if verbose:
                        print("error: {0}({1}) contains tab".format(path, lineNo))
                lineNo += 1

            data = data.replace(b"\t", b" "*indentSpaceCount) # normalize tabs to <indentSpaceCount> spaces for indent algorithm below
            lines = data.split(b"\n") # recompute lines, relies on newline normalization above

            # 3. Correct leading whitespace / bad indenting
            if checkBadIndenting:
                leadingWhitespaceRe = re.compile(b"^\s*")
                commentIsOpen = False
                previousLine = b""
                previousIndent = 0
                lineNo = 1
                for line in lines:
                    if commentIsOpen:
                        # don't check leading whitespace inside comments
                        commentIsOpen = multilineCommentIsOpenAtEol(line, commentIsOpen)
                        previousIndent = 0
                    else:
                        m = leadingWhitespaceRe.search(line)
                        indent = m.end() - m.start()
                        if indent != len(line): # ignore whitespace lines, they are considered trailing whitespace
                            if indent % indentSpaceCount != 0 and indent != previousIndent:
                                # potential bad indents are not multiples of <indentSpaceCount>,
                                # and are not indented the same as the previous line
                                s = previousLine
                                if not allowStrangeIndentOnFollowingLine(previousLine) and not allowStrangeIndentOfLine(line):
                                    status.incrementIssueCount("has-bad-indenting")
                                    if verbose or verboseBadIndenting:
                                        print("error: {0}({1}) bad indent: {2}".format(path, lineNo, indent))
                                        print(line)
                        commentIsOpen = multilineCommentIsOpenAtEol(line, commentIsOpen)
                        previousIndent = indent
                    previousLine = line
                    lineNo += 1

            # 4. No trailing whitespace
            trailingWhitespaceRe = re.compile(b"\s*$")
            lineNo = 1
            for line in lines:
                m = trailingWhitespaceRe.search(line)
                trailing = m.end() - m.start()
                if trailing > 0:
                    status.incrementIssueCount("has-trailing-whitespace")
                    if verbose:
                        print("error: {0}({1}) trailing whitespace:".format(path, lineNo))
                        print(line)
                lineNo += 1

            # 5. No non-ASCII or weird control characters
            badCharactersRe = re.compile(b"[^\t\r\n\x20-\x7E]+")
            lineNo = 1
            for line in lines:
                m = badCharactersRe.search(line)
                if m:
                    bad = m.end() - m.start()
                    if bad > 0:
                        status.incrementIssueCount("has-bad-character")
                        if verbose:
                            print("error: {0}({1}) bad character:".format(path, lineNo))
                            print(line)
                lineNo += 1

            # 6. Require EOL at EOF
            if len(data) == 0:
                status.incrementIssueCount("has-no-eol-character-at-end-of-file")
                if verbose:
                    lineNo = 1
                    print("error: {0}({1}) no end-of-line at end-of-file (empty file)".format(path, lineNo))
            else:
                lastChar = data[-1]
                if lastChar != b"\n"[0]:
                    status.incrementIssueCount("has-no-eol-character-at-end-of-file")
                    if verbose:
                        lineNo = len(lines)
                        print("error: {0}({1}) no end-of-line at end-of-file".format(path, lineNo))

            # 7. No "empty" (or whitespace) lines at end-of-file.
            # Cases:
            #   1. There is an EOL at EOF. Since the lines array is constructed by splitting on '\n',
            #      the final element in the lines array will be an empty string. This is expeced and allowed.
            #      Then continue to check for earlier empty lines.
            #   2. There is no EOF at EOL.
            #      Check for empty lines, including the final line.
            expectEmptyFinalLine = not status.hasIssue("has-no-eol-character-at-end-of-file") # i.e. we have EOL at EOF
            finalLineNo = len(lines)
            lineNo = finalLineNo
            for line in reversed(lines):
                if lineNo == finalLineNo and expectEmptyFinalLine:
                    assert len(line) == 0 # this is guaranteed, since lines = data.split('\n') and there is an EOL at EOF
                else:
                    s = line.strip(b" ") # whitespace-only-lines count as empty
                    if len(s) == 0:
                        status.incrementIssueCount("has-empty-line-at-end-of-file")
                        if verbose:
                            print("error: {0}({1}) empty line at end-of-file".format(path, lineNo))
                    else:
                        break # stop checking once we encounter a non-empty line
                lineNo -= 1


print("SUMMARY")
print("=======")
issuesFound = False
for s in statusSummary:
    if s.hasIssues():
        issuesFound = True
        print("error: " + str(s.path) + " (" + s.issueSummaryString() + ")")

if issuesFound:
    sys.exit(1)
else:
    print("all good.")
    sys.exit(0)
