import matplotlib.pyplot as plt

def read_results(filename):
    x = []
    y = []

    seen = set()
    filename = './results/' + filename

    with open(filename, "r") as f:
        for line in f:
            clean = line.strip()
            if not clean:
                continue
            parts = clean.split()
            degree = int(parts[0])
            time = float(parts[1])

            # skip duplicate degrees
            if degree in seen:
                continue

            seen.add(degree)
            x.append(degree)
            y.append(time)

    # sort by x
    x, y = zip(*sorted(zip(x, y)))
    return list(x), list(y)


def draw(datasets):
    for label, xs, ys in datasets:
        plt.plot(xs, ys, label=label, marker='o')

    plt.xscale("log", base=2)
    plt.xlabel("polynomial degree (log scale)")
    plt.ylabel("execution time (sec)")
    plt.grid(True)
    plt.title("Polynomial Multiplication")
    plt.legend()
    plt.show()

if __name__ == "__main__":
    xserial, yserial = read_results("serial.txt")
    x1, y1 = read_results("threaded_1.txt")
    x2, y2 = read_results("threaded_2.txt")
    x4, y4 = read_results("threaded_4.txt")
    x8, y8 = read_results("threaded_8.txt")

    datasets = [
        ("serial", xserial, yserial),
        ("threaded (1)", x1, y1),
        ("threaded (2)", x2, y2),
        ("threaded (4)", x4, y4),
        ("threaded (8)", x8, y8)
    ]

    draw(datasets)
