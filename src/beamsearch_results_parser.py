import pandas as pd

def analyze_heuristic_performance(input_csv_path, output_csv_path):
    try:
        df = pd.read_csv(input_csv_path)
        print(f"Successfully loaded '{input_csv_path}'")

        def get_filename_group(filename):
            return '_'.join(filename.replace('.txt', '').split('_')[:-1])

        df['filename_group'] = df['filename'].apply(get_filename_group)

        grouped_data = df.groupby(['filename_group', 'heuristic'])

        length_stats = grouped_data['length'].describe()
        time_stats = grouped_data['time_s'].describe()

        length_stats = length_stats.add_prefix('length_')
        time_stats = time_stats.add_prefix('time_s_')

        combined_stats = pd.concat([length_stats, time_stats], axis=1)

        combined_stats.to_csv(output_csv_path)
        print(f"Analysis complete. Results saved to '{output_csv_path}'")

    except FileNotFoundError:
        print(f"Error: The file '{input_csv_path}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == '__main__':
    input_file = 'results/beamsearch_results_with_h8.csv'
    output_file = 'results/beamsearch_results_with_h8_summary.csv'

    analyze_heuristic_performance(input_file, output_file)