import os
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict

import os
import pandas as pd
import matplotlib.pyplot as plt


def plot_X_Y_for_directories(directory_paths, axes_pairs,save):
    for directory_path in directory_paths:
        for entry in os.listdir(directory_path):
            entry_path = os.path.join(directory_path, entry)
            if os.path.isdir(entry_path):
                fig, ax = plt.subplots()  # Create a new figure for each directory
                for filename in os.listdir(entry_path):
                    if filename.endswith(".csv"):
                        file_path = os.path.join(entry_path, filename)
                        df = pd.read_csv(file_path)
                        for x, y in axes_pairs:
                            ax.plot(df[x], df[y], label=filename.split("_")[-1].split(".")[0])
                ax.set_xlabel(axes_pairs[0][0])  # Assuming all x axes are the same
                ax.set_ylabel(axes_pairs[0][1])
                ax.set_title(entry)
                ax.legend()
                ax.grid(True)
                if save:
                    save_path = os.path.join(entry_path, directory_path.split('/')[-1]+".png")
                    plt.savefig(save_path,format='png')  # Save the plot in the directory
                #plt.show()
                plt.close(fig)  # Close the figure to release memory


# Directory containing CSV files
w_directory = os.getcwd()
# List of directory paths
directory_paths = [os.path.join(w_directory, '2Agents')]

# Define the list of x and y axes pairs
axes_pairs = [('Gen', 'T_Max')]

# Call the function with the directory paths and axes pairs
plot_X_Y_for_directories(directory_paths, axes_pairs, True)