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
    #sns.set(style='whitegrid')
    plt.style.use('default')

    colormap = plt.cm.rainbow

    #path_prefix = 'output/image-summarization/images_500_graph/constraint_80-'
    #path_prefix = 'output/movie-recommendation/movies_graph_500/constraint_200-'
    #path_prefix = 'output/erdos-renyi/erdos_renyi-n_1000-p_50/constraint_500-'
    path_prefix = 'output/youtube-revenue/youtube_graph_1329/constraint_100-'
    #path_prefix = 'output/stochastic-block-model/stochastic_block_model-n_700-c_7-p_80-q_0/constraint_360-'

    #algorithms = ['greedy', 'random_prefix', 'random_greedy', 'random_lazy_greedy_improved-trial_1_10', 'epsilon_25-adaptive_nonmonotone_maximization', 'epsilon_25-fantom', 'epsilon_25-rounds_10-blits']
    #algorithms_clean = ['Greedy', 'Random', 'Random-Greedy', 'Random-Lazy-Greedy', 'Adaptive-Nonmonotone', 'FANTOM', 'BLITS']
    algorithms = ['greedy', 'random_prefix', 'random_lazy_greedy_improved-trial_1_10', 'epsilon_25-adaptive_nonmonotone_maximization-trial_1_10', 'epsilon_25-fantom', 'epsilon_25-rounds_10-blits']
    algorithms_clean = ['Greedy', 'Random', 'Random-Lazy-Greedy', 'Adaptive-Nonmonotone', 'FANTOM', 'BLITS']

    i = 0
    linestyles = ['-'] * len(algorithms)
    markers = ['.'] * len(algorithms)

    data = {}
    for algorithm in algorithms:
        filename = path_prefix + algorithm + '.txt'
        data[algorithm] = ReadResult(filename)

    max_x = max(data['greedy']['num_rounds'])

    for algorithm, name in zip(algorithms, algorithms_clean):
        x = data[algorithm]['num_rounds']
        y = data[algorithm]['function_values']
        if len(x) > max_x:
            x = x[:max_x]
            y = y[:max_x]
        #y = data[algorithm]['num_elements_added'][1:]
        #y = data[algorithm]['num_queries'][1:]
        #if algorithm == 'random_prefix':
        #    x = [0, max_x]
        #    y = [random_threshold, random_threshold]

        plt.plot(x, y, label=name, linestyle=linestyles[i], marker=markers[i], markevery=[len(x)-1])
        i += 1

    plt.grid()
    plt.legend(loc='lower right')
    plt.xlabel('Rounds')
    plt.ylabel('Function Value')
    x1, x2, y1, y2 = plt.axis()
    plt.axis((x1, x2, y1, y2))

    # Save figure
    new_path_prefix = path_prefix
    while new_path_prefix[-1] != '/':
        new_path_prefix = new_path_prefix[:-1]
    plt.savefig(new_path_prefix + 'tmp.pdf', bbox_inches='tight', dpi=100)

    # Show figure
    plt.show()

main()
