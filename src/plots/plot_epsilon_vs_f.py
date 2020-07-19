import matplotlib.pyplot as plt
import seaborn as sns

def ReadResult(filename):
    data = {}
    with open(filename, 'r') as f:
        fields = f.readline().split()
        for field in fields:
            data[field] = []
        for line in f:
            tokens = line.strip().split()
            for i in range(len(fields)):
                field = fields[i]
                value = eval(tokens[i])
                data[field].append(value)
    assert(len(data))
    return data

def main():
    sns.set(style='whitegrid')
    #plt.style.use('ggplot')

    #constraints = [50, 100, 150, 200, 250]
    #constraints = [20, 40, 60, 80, 100]
    #constraints = [100, 200, 300, 400, 500]
    constraint = 100
    epsilons = [0.05, 0.15, 0.25, 0.35, 0.45]

    #path_prefix = 'output/image-summarization/images_500_graph/'
    #path_prefix = 'output/movie-recommendation/movies_graph_500/'
    #path_prefix = 'output/erdos-renyi/erdos_renyi-n_1000-p_50/'
    path_prefix = 'output/youtube-revenue/youtube_graph_1329/'

    algorithms = ['random', 'random_prefix', 'greedy', 'random_greedy']
    algorithms_clean = ['Random', 'Random-Prefix', 'Greedy', 'Random-Greedy']
    data = {}
    for algorithm in algorithms:
        y = []
        filename = path_prefix + 'constraint_' + str(constraint) + '-' + algorithm + '.txt'
        tmp = ReadResult(filename)
        y.append(max(tmp['function_values']))
        print(algorithm, y)
        data[algorithm] = [y] * len(epsilons)

    for algorithm, name in zip(algorithms, algorithms_clean):
        x = epsilons 
        y = data[algorithm]
        if algorithm in ['random', 'random_prefix']:
            plt.plot(x, y, linestyle='dashed', label=name)
        else:
            plt.plot(x, y, label=name)

    adaptive_y = []
    for eps in epsilons:
        filename = path_prefix + 'constraint_' + str(constraint) + '-' + 'epsilon_' + str(int(100*eps)) + '-adaptive_nonmonotone_maximization.txt'
        tmp = ReadResult(filename)
        adaptive_y.append(max(tmp['function_values']))
    plt.plot(epsilons, adaptive_y, label='Adaptive-Nonmonotone')

    #plt.grid()
    plt.legend(loc='lower right')
    plt.xlabel('Epsilon')
    plt.ylabel('Function Value')
    x1, x2, y1, y2 = plt.axis()
    plt.axis((x1, x2, y1, y2))

    # Save figure
    new_path_prefix = path_prefix
    while new_path_prefix[-1] != '/':
        new_path_prefix = new_path_prefix[:-1]
    plt.savefig(new_path_prefix + 'tmp.pdf', bbox_inches='tight')

    # Show figure
    plt.show()

main()
