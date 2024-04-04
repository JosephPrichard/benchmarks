import csv
import sys
import re
import matplotlib.pyplot as plt

def parse_files(csv_file):
    x = []
    y = []

    with open(csv_file, 'r') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)
        for row in csv_reader:
            x.append(row[0])
            y.append(float(row[1]))

    return [x, y]

def plot_barchart(lines):

    x = []
    y = []
    
    for line in lines:
        for i in range(len(line[0])):
            if line[0][i] == "total":
                x.append(line[2])
                y.append(line[1][i])

    print(x, y)

    plt.bar(x, y, label=x)

    plt.title('Benchmark Results', color='white')
    plt.xlabel('Language', color='white')
    plt.ylabel('Time (ms)', color='white')

    plt.grid(True)

    plt.gca().set_facecolor('#333333')
    plt.xticks(color='white')
    plt.yticks(color='white')

    plt.style.use("dark_background")

    plt.legend(title='Languages')

    plt.show()

if __name__ == "__main__":
    print(sys.argv[1:])

    lines = []
    for file in sys.argv[1:]:
        name = re.search(r'bench-out-(.*).csv', file)
        line = parse_files(file)
        line.append(name.group(1))
        lines.append(line)

    plot_barchart(lines)