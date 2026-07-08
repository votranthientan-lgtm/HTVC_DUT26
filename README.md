# HTVC_DUT26
Hich-tech Vegetable Cabinet applying IOT 
```bash
Faculty of Electrical Engineering, Academic Year 2025–2026 Project Title: "High-tech vegetable cabinet applying IOT"
```
### Bảng sơ đồ nối dây MEGA 2560

| Thiết bị | Loại | Chân điều khiển trên Mega | Ghi chú |
| :--- | :--- | :--- | :--- |
| Fan T1 | Quạt | 42 | |
| Fan T2 | Quạt | 44 | |
| Fan T3 | Quạt | 46 | |
| Pump T1 | Bơm | 31, 31, 31 | Pin 31 dùng cho cả 3 tín hiệu |
| Pump T2 | Bơm | 33, 33, 33 | Pin 33 dùng cho cả 3 tín hiệu |
| Pump T3 | Bơm | 35, 35, 35 | Pin 35 dùng cho cả 3 tín hiệu |
| Led T1 | LED | 8 | |
| Led T2 | LED | 50 | |
| Led T3 | LED | 52 | |
| Sensor T1 | Cảm biến | A1, 7 | A1 (Analog), D7(DHT21) |
| Sensor T2 | Cảm biến | 0, 6 | 0 (RX), D6(DHT21) |
| Sensor T3 | Cảm biến | 0, 5 | 0 (RX), D5(DHT21) |
| pH Sensor | pH/Temp | A0, 13 | A0 (pH), D13(TEMP WATER) |
| Peri Pump | Bơm nhu động | 9, 10, 11, 12 | |
| Third Pump | Bơm 3 | 22, 24 | |
| LCD T1 | Màn hình | 0x27 (I2C) | SDA/SCL |
| LCD T2 | Màn hình | 0x26 (I2C) | SDA/SCL |
| LCD T3 | Màn hình | 0x25 (I2C) | SDA/SCL |
| Menu Control | Menu/Nút | 0x24 (I2C), 41, 43, 45, 47 | I2C + 4 chân Digital |

***Hình ảnh sản phẩm
<img width="1920" height="2560" alt="image" src="https://github.com/user-attachments/assets/909a3a1e-39d1-4353-b07c-1f6b864a8187" />

***Giao diện control trên LCD bằng nút bấm
<img width="2568" height="1926" alt="image" src="https://github.com/user-attachments/assets/f6e82875-14be-4b0f-98b6-df89e8186733" />

***Giao diện web dashboard
<img width="1280" height="720" alt="image" src="https://github.com/user-attachments/assets/52312c2b-f3ed-40ae-9cad-918821d5e4ed" />
<img width="1280" height="730" alt="image" src="https://github.com/user-attachments/assets/2d58d695-e9ad-40e2-afdb-ce833b4656e1" />
<img width="1280" height="718" alt="image" src="https://github.com/user-attachments/assets/56cda728-b53b-458c-9cd2-f7a2fe01e271" />


