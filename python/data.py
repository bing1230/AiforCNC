import pandas as pd

# 指定檔案路徑
file_path = r"11111\實驗數據\有泡震\1.csv"

# 讀取 CSV 檔案
df = pd.read_csv(file_path)

# 印出資料
print(df)
