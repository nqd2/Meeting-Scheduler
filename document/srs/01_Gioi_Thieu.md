# 1. GIỚI THIỆU TỔNG QUAN

## 1.1. Mục đích
Tài liệu này mô tả các yêu cầu kỹ thuật và chức năng cho **Ứng dụng Quản lý Cuộc họp (Meeting Scheduler)**. Hệ thống giúp kết nối sinh viên và giáo viên trong việc đặt lịch hẹn, quản lý thời gian và lưu trữ biên bản cuộc họp.

## 1.2. Phạm vi sản phẩm
Ứng dụng bao gồm:
- **Client**: Ứng dụng Desktop đa nền tảng (xây dựng bằng **Electron** + React/Web technologies).
- **Server**: Máy chủ xử lý trung tâm (xây dựng bằng ngôn ngữ **C**).
- **Giao thức**: Giao tiếp thời gian thực qua **WebSocket Secure (wss://)**.
- **Cơ sở dữ liệu**: Lưu trữ dữ liệu phi cấu trúc với **MongoDB**.

## 1.3. Đối tượng sử dụng
Hệ thống phục vụ 2 nhóm người dùng chính:
1.  **Giáo viên (Teacher)**:
    - Khai báo các khe thời gian rảnh (Slots).
    - Quản lý lịch hẹn với sinh viên.
    - Ghi và xem lại biên bản cuộc họp.
2.  **Sinh viên (Student)**:
    - Xem lịch rảnh của giáo viên.
    - Đặt lịch hẹn (cá nhân hoặc nhóm).
    - Theo dõi lịch sử cuộc họp.


