import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import matplotlib as mpl

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

    constraint = 40
    epsilon = 25
    num_thresholds = 92
    algorithm = 'adaptive_nonmonotone_maximization'

    #path_prefix = 'output/image-summarization/images_500_graph/'
    #path_prefix = 'output/movie-recommendation/movies_graph_500/'
    #path_prefix = 'output/erdos-renyi/erdos_renyi-n_1000-p_50/'
    path_prefix = 'output/youtube-revenue/youtube_graph_1329/'

    path_prefix += 'constraint_' + str(constraint) + '-'
    path_prefix += 'epsilon_' + str(epsilon) + '-'
    path_prefix += 'threshold_'

    colormap = plt.cm.rainbow

    data = {}
    for i in range(num_thresholds):
        if i % 2 != 0: continue
        path = path_prefix
        path += str(i) + '_' + str(num_thresholds) + '-'
        path += algorithm + '.txt'
        tmp = ReadResult(path)
        x = tmp['num_rounds']
        y = tmp['function_values']
        data[i] = (x, y)
        plt.plot(x, y, marker='.', color=colormap(float(i)/num_thresholds), label=str(i), cmap=colormap)

    #plt.grid()
    #plt.colorbar(colormap)
    #plt.legend(loc='lower right')
    plt.xlabel('Rounds')
    plt.ylabel('Function Value')
    x1, x2, y1, y2 = plt.axis()
    plt.axis((x1, x2, y1, y2))

    # Save figure
    new_path_prefix = path_prefix
    while new_path_prefix[-1] != '/':
        new_path_prefix = new_path_prefix[:-1]
    plt.savefig(new_path_prefix + 'tmp.pdf', bbox_inches='tight')

    plt.show()

main()
