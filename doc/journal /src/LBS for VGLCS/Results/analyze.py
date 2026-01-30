import pandas as pd
import re
import os

# ---- Учитај фајлове ----
df1 = pd.read_csv("basic_imsbs.csv")  
df2 = pd.read_csv("second_file.csv")  

# ---- Извуци 4 броја из имена фајла ----
def extract_numbers(filename):
    # узимамо сва 4 броја из имена: n_m_k_index
    m = re.match(r".*?_(\d+)_(\d+)_(\d+)_(\d+)\.txt\.out", filename)
    if m:
        return int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4))
    else:
        return None, None, None, None

for df in [df1, df2]:
    df[['n','m','k','idx']] = df['file'].apply(lambda x: pd.Series(extract_numbers(x)))
    # kreiramo key po prva tri broja
    df['key'] = df[['n','m','k']].astype(str).agg('_'.join, axis=1)

# ---- Spoji podatke po key ----
merged = pd.merge(df1, df2, on='key', suffixes=('_1','_2'))

# ---- Agregiraj po prva tri broja (key) ----
agg = merged.groupby('key').agg({
    'time_1': 'mean',
    'quality_1': 'mean',
    'time_2': 'mean',
    'quality_2': 'mean'
}).reset_index()

# ---- Rezultat ----
print(agg)

# ---- Sačuvaj u CSV ----
agg.to_csv("merged_aggregated.csv", index=False)

