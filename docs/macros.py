import os

def declare_variables(variables, macro):
    @macro
    def code_snippet(filename: str, snippet: str = "", language: str = "cpp"):
        """
        Load code from a file and save as a preformatted code block.
        If a language is specified, it's passed in as a hint for syntax highlighters.
        Example usage in markdown:
            {{code_from_file("code/myfile.py", "python")}}
        """
        docs_dir = variables.get("docs_dir", "docs")

        # Look for file
        abs_docs_path = os.path.abspath(os.path.join(docs_dir, filename))
        abs_root_path = os.path.abspath(os.path.join(docs_dir, "..", filename))
        abs_examples_path = os.path.abspath(os.path.join(docs_dir, "../examples/", filename))
        abs_path = ''
        if os.path.exists(abs_docs_path):
            abs_path = abs_docs_path
        elif os.path.exists(abs_root_path):
            abs_path = abs_root_path
        elif os.path.exists(abs_examples_path):
            abs_path = abs_examples_path

        # File not found
        if not os.path.exists(abs_path):
            return f"""<b>File not found: {filename}</b>"""

        # Read snippet from file
        with open(abs_path, "r") as f:
            if not snippet:
                return (
                    f"""```{language}\n{f.read()}\n```"""
                )
            else:
                # Extract the snippet
                contents = f.read()
                start_pos = contents.find('//[' + snippet)
                if start_pos == -1:
                    return f"""<b>Snippet {snippet} not found in {filename}</b>"""
                end_pos = contents.find('//]', start_pos)
                if end_pos == -1:
                    return f"""<b>Snippet {snippet} not found in {filename}</b>"""
                contents = contents[(start_pos + 3 + len(snippet)):(end_pos - 3)]

                # Identify snippet header
                content_lines = contents.splitlines()
                first_line = content_lines[0]
                header = ''
                if not first_line.isspace() and not len(first_line) == 0:
                    header = first_line.strip()

                # Identify indent
                indent_size = 20
                for line in content_lines[1:]:
                    if not line.isspace() and not len(line) == 0:
                        first_char_pos = len(line) - len(line.lstrip())
                        indent_size = min(indent_size, first_char_pos)

                # Construct snippet
                contents = ''
                if len(header) != 0:
                    contents += '=== "' + header + '"\n\n    '
                contents += '```' + language
                if len(content_lines) > 10:
                    contents += ' linenums="1" '
                contents += '\n'
                for line in content_lines[1:]:
                    if len(header) != 0:
                        contents += '    '
                    contents += line[indent_size:] + '\n'
                if len(header) != 0:
                    contents += '    '
                contents += '```\n'
                return contents