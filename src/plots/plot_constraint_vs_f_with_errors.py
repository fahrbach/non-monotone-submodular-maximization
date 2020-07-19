import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

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
    plt.style.use('default')

    #constraints = [50, 100, 150, 200, 250]
    constraints = [20, 40, 60, 80, 100]
    #constraints = [100, 200, 300, 400, 500]

    #path_prefix = 'output/image-summarization/images_500_graph/'
    #path_prefix = 'output/movie-recommendation/movies_graph_500/'
    #path_prefix = 'output/erdos-renyi/erdos_renyi-n_1000-p_50/'
    path_prefix = 'output/youtube-revenue/youtube_graph_1329/'

    #algorithms = ['greedy', 'random_prefix', 'random_greedy', 'random_lazy_greedy_improved', 'epsilon_25-adaptive_nonmonotone_maximization', 'epsilon_25-fantom', 'epsilon_25-rounds_10-blits']
    #algorithms_clean = ['Greedy', 'Random', 'Random-Greedy', 'Random-Lazy-Greedy', 'Adaptive-Nonmonotone', 'FANTOM', 'BLITS']
    algorithms = ['greedy', 'random_prefix', 'random_lazy_greedy_improved', 'epsilon_25-adaptive_nonmonotone_maximization', 'epsilon_25-fantom', 'epsilon_25-rounds_10-blits']
    algorithms_clean = ['Greedy', 'Random', 'Random-Lazy-Greedy', 'Adaptive-Nonmonotone', 'FANTOM', 'BLITS']


    num_trials = {}
    num_trials['Random'] = 10
    num_trials['Random-Greedy'] = 10
    num_trials['Random-Lazy-Greedy'] = 10
    num_trials['Adaptive-Nonmonotone'] = 10
    num_trials['BLITS'] = 3  # May change depending on dataset

    data = {}
    errors = {}
    for algorithm, name in zip(algorithms, algorithms_clean):
        if name in ['Greedy', 'FANTOM']:
            y = []
            for c in constraints:
                filename = path_prefix + 'constraint_' + str(c) + '-' + algorithm + '.txt'
                tmp = ReadResult(filename)
                y.append(max(tmp['function_values']))
            data[algorithm] = y
        else:  # get error bars
            TRIALS = num_trials[name]
            y = []
            std_dev = []
            for c in constraints:
                function_values = []
                for trial in range(1, TRIALS + 1):
                    filename = path_prefix + 'constraint_' + str(c) + '-' + algorithm + '-trial_' + str(trial) + '_' + str(TRIALS) + '.txt'
                    tmp = ReadResult(filename)
                    function_value = max(tmp['function_values'])
                    function_values.append(function_value)
                y.append(np.mean(function_values))
                std_dev.append(np.std(function_values))
            data[algorithm] = y
            errors[algorithm] = std_dev

    i = 0
    linestyles = ['-'] * len(algorithms)
    markers = ['.'] * len(algorithms)
    for algorithm, name in zip(algorithms, algorithms_clean):
        x = constraints
        y = data[algorithm]
        if name in ['Greedy', 'FANTOM']:
            plt.errorbar(x, y, linestyle=linestyles[i], marker=markers[i], label=name)
        else:
            error_bars = errors[algorithm]
            plt.errorbar(x, y, yerr=error_bars, capsize=2, linestyle=linestyles[i], marker=markers[i], label=name)
        i += 1

    plt.grid()
    plt.legend(loc='lower right')
    plt.xlabel('Cardinality Constraint')
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
