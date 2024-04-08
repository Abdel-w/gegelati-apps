import os
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict

# Set the backend to a different one that supports interactive display
plt.switch_backend('TkAgg')


def plot_X_Y(x,y):
    # Directory containing CSV files
    csv_directory = os.getcwd()

    # Dictionary to store sum of y values and count of occurrences for each x value
    x_y_sum_count = defaultdict(lambda: [0, 0])  # Key: x value, Value: [sum of y values, count]
    print(csv_directory)
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
                    # Group y values by x values and update the dictionary
                    for x_val, y_val in zip(df[x], df[y]):
                        x_y_sum_count[x_val][0] += y_val
                        x_y_sum_count[x_val][1] += 1

    # Calculate the average of y values for each x value
    avg_y_values = {x_val: y_sum / count for x_val, (y_sum, count) in x_y_sum_count.items()}

    # Sort the dictionary by x values
    sorted_avg_y_values = dict(sorted(avg_y_values.items()))

    # Plot the curve of y = f(x)
    plt.plot(list(sorted_avg_y_values.keys()), list(sorted_avg_y_values.values()), label=f'{y} = f({x})')

    plt.xlabel(x)
    plt.ylabel(y)
    plt.title(f'{y} vs {x}')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(csv_directory, f'{y}_vs_{x}.png'))
    plt.show()


plot_X_Y('Gen','T_Max')