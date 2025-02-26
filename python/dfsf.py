import pandas as pd

# 指定檔案路徑
file_path = r"11111\實驗數據\有泡震\1.csv"

# 讀取 CSV 檔案
df = pd.read_csv(file_path)

# 顯示讀取的數據
print(df)


import matplotlib.pyplot as plt

# 繪製圖表
plt.plot(df["time"], df["x"], label="X")
plt.plot(df["time"], df["y"], label="Y")
plt.plot(df["time"], df["z"], label="Z")

# 添加標題和標籤
plt.title("XYZ Position Over Time")
plt.xlabel("Time")
plt.ylabel("Position")

# 顯示圖例
plt.legend()

# 顯示圖表
plt.show()
