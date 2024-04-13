import os
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict


def plot_X_Y_for_directories(directory_paths, axes_pairs):
    # Iterate over each directory
    for entry_path in directory_paths:
        # Initialize dictionary to store sum of y values and count of occurrences for each x value for each axis pair
        x_y_sum_count = {}
        for x, y in axes_pairs:
            x_y_sum_count[(x, y)] = defaultdict(lambda: [0, 0])  # Key: (x, y), Value: defaultdict [sum of y values, count]
        # Iterate over all CSV files in the directory
        for dir_i in os.listdir(entry_path):
            # Join the parent directory path with the current entry to get the full path
            dir_i = os.path.join(entry_path, dir_i)
            # Check if the entry is a directory
            if os.path.isdir(dir_i):
                # Iterate over all CSV files in the directory
                for filename in os.listdir(dir_i):
                    if filename.endswith(".csv"):
                        file_path = os.path.join(dir_i, filename)
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
            plt.plot(list(sorted_avg_y_values.keys()), list(sorted_avg_y_values.values()), label=dir_i.split("/")[-2])

    plt.xlabel(axes_pairs[0][0])  # Assuming all x axes are the same
    plt.ylabel(axes_pairs[0][1])
    plt.title('Multiple Curves Plot')
    plt.legend()
    plt.grid(True)
    save_path = os.path.join(entry_path, directory_paths[0]+".png")
    plt.savefig(save_path)  # Save the plot as a PNG file
    plt.show()


# Directory containing CSV files
w_directory = os.getcwd()
# List of directory paths containing CSV files
dir_paths = [os.path.join(w_directory, "FL/5Agents"), os.path.join(w_directory, "classic")]
#,  os.path.join(w_directory, "FL/3Agents"),os.path.join(w_directory, "FL/4Agents"),  os.path.join(w_directory, "FL/5Agents")]


# Define the list of x and y axes pairs
axes_pairs = [('Gen', 'T_Max')]
# Call the function with the list of directory paths and x, y axes
plot_X_Y_for_directories(dir_paths, axes_pairs)
