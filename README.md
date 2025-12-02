# Meeting Scheduler Application

Ứng dụng quản lý lịch họp giữa Sinh viên và Giáo viên, phục vụ môn học IT4062 - Thực hành Lập trình mạng.

## 👨‍💻 Nhóm thực hiện (Nhóm 12)
- **Vũ Trần Tuấn Minh** - 20225891
- **Nguyễn Trần Thái Dương** - 20225822
- **Nguyễn Quý Đức** - 20235682

## 📖 Giới thiệu
Hệ thống giúp giải quyết vấn đề đặt lịch hẹn giữa sinh viên và giáo viên một cách tự động và hiệu quả.
- **Giáo viên**: Khai báo các khe thời gian rảnh, quản lý yêu cầu hẹn, ghi biên bản cuộc họp.
- **Sinh viên**: Xem lịch rảnh của giáo viên, đặt lịch hẹn (cá nhân hoặc nhóm), theo dõi lịch sử.

## 🛠 Công nghệ sử dụng

| Thành phần | Công nghệ | Mô tả |
|------------|-----------|-------|
| **Client** | Electron + React | Ứng dụng Desktop đa nền tảng, giao diện người dùng hiện đại. |
| **Server** | C Language | Xử lý trung tâm, hiệu năng cao, quản lý kết nối đa luồng. |
| **Database** | MongoDB | Cơ sở dữ liệu NoSQL lưu trữ Users, Slots, Meetings, Minutes. |
| **Protocol** | WebSocket (WSS) | Giao thức truyền tải thời gian thực. Định dạng bản tin Text-based. |

## 📂 Cấu trúc dự án

```
Meeting-Scheduler/
├── c-server/           # Mã nguồn Server (C)
│   └── server.c        # Entry point của server
├── electron-dashboard/ # Mã nguồn Client (Electron/React)
│   ├── src/            # Source code frontend
│   └── electron/       # Main process code
├── document/           # Tài liệu dự án
│   ├── IT4060.pdf      # Đề bài/Báo cáo gốc
│   └── srs/            # Đặc tả yêu cầu phần mềm (SRS)
└── README.md           # File hướng dẫn này
```

## 🚀 Hướng dẫn cài đặt & Chạy

### Yêu cầu tiên quyết (Prerequisites)
- **C Compiler**: GCC hoặc Clang.
- **Node.js**: Phiên bản 16+ & npm/yarn.
- **MongoDB**: Đã cài đặt và đang chạy service.
- **Libraries (C)**:
  - `mongo-c-driver` (libmongoc, libbson)
  - WebSocket library (e.g., `libwebsockets` hoặc tương đương được dùng trong server)

### 1. Cấu hình Database (MongoDB Atlas)
Dự án sử dụng MongoDB Online (Atlas) thay vì Localhost để đảm bảo tính sẵn sàng và dễ dàng triển khai.

1. Tạo tài khoản và Cluster mới tại [MongoDB Atlas](https://www.mongodb.com/atlas/database).
2. Trong phần **Database Access**, tạo một user mới (username/password).
3. Trong phần **Network Access**, cho phép IP hiện tại (hoặc `0.0.0.0/0` để test) truy cập.
4. Lấy Connection String (Driver C - libmongoc), ví dụ:
   `mongodb+srv://<username>:<password>@cluster0.example.mongodb.net/?retryWrites=true&w=majority`
5. Cấu hình chuỗi kết nối này vào mã nguồn Server C (biến môi trường hoặc file config `config.h`).

### 2. Khởi chạy Server
```bash
cd c-server
# Lệnh biên dịch mẫu (cần điều chỉnh tùy theo thư viện thực tế sử dụng)
gcc server.c -o server $(pkg-config --cflags --libs libmongoc-1.0 libbson-1.0) -lwebsockets

# Chạy server
./server
```

### 3. Khởi chạy Client
```bash
cd electron-dashboard

# Cài đặt dependencies
yarn install
# hoặc
yarn

# Chạy chế độ phát triển (Development)
yarn electron:dev

# Hoặc build và chạy ứng dụng
yarn electron:build
```

## 📄 Tài liệu chi tiết
Xem chi tiết đặc tả kỹ thuật trong thư mục `document/srs/`:
- [Giới thiệu](document/srs/01_Gioi_Thieu.md)
- [Kiến trúc hệ thống](document/srs/02_Kien_Truc_He_Thong.md)
- [Yêu cầu chức năng](document/srs/03_Yeu_Cau_Chuc_Nang.md)
- [Cơ sở dữ liệu](document/srs/04_Co_So_Du_Lieu.md)
- [Giao thức bản tin](document/srs/05_Giao_Thuc_Ban_Tin.md)

