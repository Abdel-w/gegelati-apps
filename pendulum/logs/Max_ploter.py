import os
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict


def plot_X_Y_for_directories(directory_paths, axes_pairs, save=False):
    # Iterate over each directory
    for entry_path in directory_paths:
        # Initialize dictionary to store maximum y values for each x value for each axis pair
        max_y_values = defaultdict(dict)

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
                        # Update max_y_values with maximum y values for each x value
                        for x, y in axes_pairs:
                            max_y_values[(x, y)].update({x_val: max(max_y_values[(x, y)].get(x_val, float('-inf')), y_val)
                                                         for x_val, y_val in zip(df[x], df[y])})

        # Plot the curves for each axis pair on the same figure
        for x, y in axes_pairs:
            sorted_max_y_values = dict(sorted(max_y_values[(x, y)].items()))
            plt.plot(list(sorted_max_y_values.keys()), list(sorted_max_y_values.values()), label=entry_path.split("/")[-1])

    plt.xlabel(axes_pairs[0][0])  # Assuming all x axes are the same
    plt.ylabel(axes_pairs[0][1])
    plt.title('Multiple Curves Plot')
    plt.legend()
    plt.grid(True)
    if save:
        save_path = os.path.join(entry_path, directory_paths[0] + ".png")
        plt.savefig(save_path)  # Save the plot as a PNG file
    plt.show()


# Directory containing CSV files
w_directory = os.getcwd()
# List of directory paths containing CSV files
dir_paths = [
    os.path.join(w_directory, "FL/2Agents"),
    os.path.join(w_directory, "FL/3Agents"),
    os.path.join(w_directory, "FL/4Agents"),
    os.path.join(w_directory, "FL/5Agents"),
    os.path.join(w_directory, "classic")
]

# Define the list of x and y axes pairs
axes_pairs = [('Gen', 'T_Max')]
# Call the function with the list of directory paths and x, y axes
plot_X_Y_for_directories(dir_paths, axes_pairs, False)
