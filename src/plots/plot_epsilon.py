import matplotlib.pyplot as plt

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
    #path_prefix = 'output/image-summarization/images_500_graph/constraint_80-'
    path_prefix = 'output/movie-recommendation/movies_graph_500/constraint_50-'
    #path_prefix = 'output/erdos-renyi/erdos_renyi-n_1000-p_50/constraint_700-'
    #path_prefix = 'output/stochastic-block-model/stochastic_block_model-n_700-c_7-p_80-q_0/constraint_360-'

    algorithm_epsilon_pairs = [
        ['adaptive_nonmonotone_maximization', 'AdaptiveNonmonotone', [50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 1]],
        ['adaptive_maximization', 'ThresholdSampling', [50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 1]],
        ['rounds_10-blits', 'BLITS', [50, 25]]
    ]
    data = {}
    for l in algorithm_epsilon_pairs:
        algorithm = l[0]
        clean = l[1]
        epsilons = l[2]
        data[algorithm] = {}
        data[algorithm]['num_rounds'] = []
        data[algorithm]['function_value'] = []
        data[algorithm]['epsilons'] = []
        data[algorithm]['clean'] = clean
        for eps in epsilons:
            filename = path_prefix + 'epsilon_' + str(eps) + '-' + algorithm + '.txt'
            tmp = ReadResult(filename)
            function_value = max(tmp['function_values'])
            num_rounds = max(tmp['num_rounds'])
            data[algorithm]['num_rounds'].append(num_rounds)
            data[algorithm]['function_value'].append(function_value)
            data[algorithm]['epsilons'].append(eps)
            print(algorithm, eps, num_rounds, function_value)

    for algorithm in data:
        x = data[algorithm]['epsilons']
        greedy_x = [_ for _ in x]
        y = data[algorithm]['function_value']
        print(algorithm, x, y)
        plt.plot(x, y, label=data[algorithm]['clean'])

    # Get greedy baseline
    filename = path_prefix + 'greedy.txt'
    tmp = ReadResult(filename)
    greedy_value = max(tmp['function_values'])
    greedy_y = [greedy_value for _ in greedy_x]
    plt.plot(greedy_x, greedy_y, label='Greedy')

    # Get random baseline
    filename = path_prefix + 'random.txt'
    tmp = ReadResult(filename)
    greedy_value = max(tmp['function_values'])
    greedy_y = [greedy_value for _ in greedy_x]
    plt.plot(greedy_x, greedy_y, label='Random')

    # Get random greedy baseline
    filename = path_prefix + 'random_greedy.txt'
    tmp = ReadResult(filename)
    greedy_value = max(tmp['function_values'])
    greedy_y = [greedy_value for _ in greedy_x]
    plt.plot(greedy_x, greedy_y, label='RandomGreedy')

    plt.grid()
    plt.legend()
    plt.xlabel('Rounds')
    plt.ylabel('Function Value')
    x1, x2, y1, y2 = plt.axis()
    plt.axis((x1, x2, y1, y2))
    plt.show()

main()
