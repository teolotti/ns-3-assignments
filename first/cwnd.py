import matplotlib.pyplot as plt

def parse_cwnd_file(filename):
    times1 = []
    cwnds1 = []
    times2 = []
    cwnds2 = []

    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            socket_id = parts[0]
            time = float(parts[1])
            cwnd = int(parts[2])
            
            if socket_id == "1":
                times1.append(time)
                cwnds1.append(cwnd)
            elif socket_id == "2":
                times2.append(time)
                cwnds2.append(cwnd)

    return times1, cwnds1, times2, cwnds2

def plot_cwnd(times1, cwnds1, times2, cwnds2):
    plt.figure(figsize=(10, 5))
    
    plt.plot(times1, cwnds1, label='Socket 1')
    plt.plot(times2, cwnds2, label='Socket 2')
    
    plt.xlabel('Time (s)')
    plt.ylabel('Congestion Window Size (bytes)')
    plt.title('Congestion Window Size over Time')
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    times1, cwnds1, times2, cwnds2 = parse_cwnd_file("/home/matteo/ns-3-dev/scratch/ns-3-first-assignment/first/first-assignment-tcp-cwnd.txt")
    plot_cwnd(times1, cwnds1, times2, cwnds2)
