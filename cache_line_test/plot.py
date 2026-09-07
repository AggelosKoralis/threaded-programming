import matplotlib.pyplot as plt

def read_results(file):
    x = []
    y = []

    f = open(file, "r")
    for line in f:
        clean = line.strip()
        parts = clean.split()
        x.append(int(parts[0]))
        y.append(float(parts[1]))

    # sort by x
    x, y = zip(*sorted(zip(x, y)))
    return list(x), list(y)

def draw(xserial, yserial, xthreaded, ythreaded):
    plt.plot(xserial, yserial, label="serial", marker='o')
    plt.plot(xthreaded, ythreaded, label="threaded", marker='o')
    plt.xlabel("array length")
    plt.ylabel("time (sec)")
    plt.title("cache line exercise")
    plt.legend()

    plt.xscale("log")
    plt.grid(True)

    plt.show()

if __name__ == "__main__":
    xserial, yserial = read_results("serial.txt")
    xthreaded, ythreaded = read_results("threaded.txt")

    draw(xserial, yserial, xthreaded, ythreaded)
