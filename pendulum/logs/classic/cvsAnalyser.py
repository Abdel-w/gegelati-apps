import os
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict


def plot_X_Y(axes_pairs):
    # Directory containing CSV files
    csv_directory = os.getcwd()

    # Initialize dictionary to store sum of y values and count of occurrences for each x value for each axis pair
    x_y_sum_count = {}
    for x, y in axes_pairs:
        x_y_sum_count[(x, y)] = defaultdict(lambda: [0, 0])  # Key: (x, y), Value: defaultdict [sum of y values, count]

    # Iterate over each entry in the parent directory
    for entry in os.listdir(csv_directory):
        # Join the parent directory path with the current entry to get the full path
        entry_path = os.path.join(csv_directory, entry)
        # Check if the entry is a directory
        if os.path.isdir(entry_path):
            # Iterate over all CSV files in the directory
            for filename in os.listdir(entry_path):
                if filename.endswith(".csv"):
                    file_path = os.path.join(entry_path, filename)
                    # Read CSV file into a DataFrame
                    df = pd.read_csv(file_path)
                    # Group y values by x values and update the dictionary for each axis pair
                    for x, y in axes_pairs:
                        for x_val, y_val in zip(df[x], df[y]):
                            x_y_sum_count[(x, y)][x_val][0] += y_val
                            x_y_sum_count[(x, y)][x_val][1] += 1

    # Calculate the average of y values for each x value for each axis pair
    avg_y_values = {}
    for x, y in axes_pairs:
        avg_y_values[(x, y)] = {x_val: y_sum / count for x_val, (y_sum, count) in x_y_sum_count[(x, y)].items()}

    # Plot the curves for each axis pair on the same figure
    for x, y in axes_pairs:
        sorted_avg_y_values = dict(sorted(avg_y_values[(x, y)].items()))
        plt.plot(list(sorted_avg_y_values.keys()), list(sorted_avg_y_values.values()), label=f'{y} = f({x})')

    plt.xlabel(axes_pairs[0][0])  # Assuming all x axes are the same
    plt.ylabel('Y')
    plt.title('Multiple Curves Plot')
    plt.legend()
    plt.grid(True)
    plt.show()


# Define the list of x and y axes pairs
axes_pairs = [('Gen', 'T_Max')]  # Add more pairs as needed

# Call the function with the list of axes pairs
plot_X_Y(axes_pairs)
