"""
This script reads the most recent Excel file from a specified Google Drive folder,
plots impedance vs frequency, and saves the plot as a PDF.
"""
import os
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

def find_recent_excel_file(directory_path):
    """Finds the most recent Excel file in the specified directory."""

    directory = Path(directory_path)

    # Filter for Excel files
    excel_files = [f for f in directory.glob("*.xlsx") if f.is_file()]

    if not excel_files:
        return None

    # Find the file with the latest modification time
    return max(excel_files, key=os.path.getmtime)

def plot():
    """
    This function plots data from read.serial.py and then saves the file and plot 
    to Google Drive.
    """
    user_input = input("Enter the name or description to include in the plot title:" + " ")

    # Create a Path object for your directory
    directory_path = Path(
        '/Users/jeanetteqi/Library/CloudStorage/GoogleDrive-jeanette.qi@sjsu.edu/'
        'Shared drives/MS project_Bryant and Sejad_Human Milk'
        ' for Premature Infants/Prototype_MilkSensor_CSVData'
        )
    # Check if path exists
    if os.path.exists(directory_path):
        print("Excel files found")
    else:
        print('No Excel files found')

    try:
        # List all Excel files in folder
        recent_file = find_recent_excel_file(directory_path)
        # Load the data from the most recent file
        df = pd.read_excel(recent_file, header=0)


    except Exception as e:
        print(f"An error occurred: {e}")
        return

    # Plotting the data with parameters
    plt.figure(figsize=(10, 5))
    plt.plot(
        df['c_frequency'] * 1000, #Convert kHz to Hz
        df['impedance'],
        marker='o',
        linestyle='-',
        color='b',
        label='Impedance vs Frequency'
    )

    # Adding titles and labels
    plt.title(f"Frequency vs Impedance of {user_input}")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Impedance (ohms)")

    # Rotating the x-axis labels for better readability
    plt.xticks(rotation=45)
    plt.minorticks_on()
    plt.tight_layout()  # Adjust layout to prevent clipping of tick labels

    # Save the plot as PDF. Rename file with the file without the path.
    file_base = os.path.splitext(os.path.basename(recent_file))[0]
    save_plot = os.path.join(directory_path, f"{file_base}_{user_input}_plot.pdf")
    plt.savefig(save_plot, format="pdf")
    print(f"Plot saved at: {save_plot}")

    plt.autoscale()  # Scale according to data
    plt.show()  # Display the plot


if __name__ == "__main__":
    plot()
    