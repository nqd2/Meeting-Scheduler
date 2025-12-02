# 3. YÊU CẦU CHỨC NĂNG

## 3.1. Quản lý tài khoản (Chung)

### 3.1.1. Đăng ký (Register)
- **Mô tả**: Người dùng mới tạo tài khoản.
- **Input**: Username, Password.
- **Xử lý**: Kiểm tra username đã tồn tại trong MongoDB chưa. Nếu chưa, tạo mới.
- **Output**: Thành công (kèm Token) hoặc Thất bại (Lỗi trùng tên).

### 3.1.2. Đăng nhập (Login)
- **Mô tả**: Xác thực người dùng vào hệ thống.
- **Input**: Username, Password.
- **Xử lý**: So khớp username/password trong DB.
- **Output**: Token phiên làm việc và Role (Teacher/Student).

### 3.1.3. Đăng xuất (Logout)
- **Mô tả**: Kết thúc phiên làm việc.
- **Xử lý**: Vô hiệu hóa Token hiện tại (Xóa khỏi danh sách active sessions trên server).

---

## 3.2. Chức năng cho Giáo viên (Teacher)

### 3.2.1. Quản lý khe thời gian (Slot Management)
- **Thêm Slot (ADD_SLOT)**: Khai báo thời gian rảnh.
    - *Input*: Thời gian bắt đầu, Thời gian kết thúc.
    - *Quy tắc*: Không được trùng với slot đã có.
- **Sửa Slot (UPDATE_SLOT)**: Điều chỉnh thời gian.
- **Xóa Slot (DELETE_SLOT)**: Hủy bỏ khe thời gian rảnh.
- **Xem danh sách (LIST_FREE_SLOTS)**: Xem các slot mình đã tạo.

### 3.2.2. Quản lý cuộc họp & Biên bản
- **Xem lịch hẹn (LIST_APPOINTMENTS)**: Xem danh sách các slot đã được sinh viên đặt (booked slots).
- **Ghi biên bản (ADD_MINUTES)**:
    - *Mô tả*: Lưu nội dung tóm tắt cuộc họp sau khi kết thúc.
    - *Lưu trữ*: Nội dung biên bản được lưu trực tiếp vào document Meeting trong MongoDB (thay vì file .txt riêng lẻ như thiết kế cũ).
- **Xem biên bản (GET_MINUTES)**: Truy xuất nội dung biên bản cũ.
- **Xem lịch sử sinh viên (VIEW_HISTORY)**: Xem danh sách các cuộc họp quá khứ của một sinh viên cụ thể.

---

## 3.3. Chức năng cho Sinh viên (Student)

### 3.3.1. Đặt lịch (Booking)
- **Xem lịch rảnh (LIST_FREE_SLOTS)**: Tìm kiếm các slot rảnh của giáo viên.
- **Đặt lịch cá nhân (BOOK_INDIVIDUAL)**:
    - *Input*: Teacher ID, Slot ID.
    - *Xử lý*: Chuyển trạng thái Slot từ "Free" sang "Booked". Gắn Student ID vào Meeting.
- **Đặt lịch nhóm (BOOK_GROUP)**:
    - *Input*: Teacher ID, Slot ID, Danh sách Member IDs.
    - *Xử lý*: Tương tự đặt lịch cá nhân nhưng lưu danh sách nhiều sinh viên.

### 3.3.2. Quản lý lịch cá nhân
- **Hủy lịch (CANCEL_MEETING)**: Hủy cuộc họp đã đặt (trước thời điểm diễn ra).
- **Xem lịch tuần (LIST_WEEK_MEETINGS)**: Xem danh sách các cuộc họp của bản thân trong tuần.

