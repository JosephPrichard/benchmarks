import sys

def compare_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        lines1 = f1.readlines()
        lines2 = f2.readlines()

        min_lines = min(len(lines1), len(lines2))

        for i in range(min_lines):
            l1 = lines1[i].strip()
            l2 = lines2[i].strip()
            if l1 != l2:
                print(f"Difference found at line {i+1}:")
                print(f"    {file1}: {l1}")
                print(f"    {file2}: {l2}")
        
        if len(lines1) > len(lines2):
            print(f"Additional lines in {file1}:")
            for i in range(min_lines, len(lines1)):
                print(f"    {file1}: {lines1[i].strip()}")
        elif len(lines1) < len(lines2):
            print(f"Additional lines in {file2}:")
            for i in range(min_lines, len(lines2)):
                print(f"    {file2}: {lines2[i].strip()}")


def main(files):
    for i in range(len(files) - 1):
        file1 = files[i]
        file2 = files[i + 1]
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            if f1.read() == f2.read():
                print(f"Contents of {file1} and {file2} are equal")
            else:
                print(f"Contents of {file1} and {file2} are not equal")
                compare_files(file1, file2)


if __name__ == "__main__":
    main(sys.argv[1:])