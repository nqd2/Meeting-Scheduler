# 5. GIAO THỨC BẢN TIN (PROTOCOL)

Mặc dù sử dụng WebSocket làm giao thức truyền tải (Transport Layer), định dạng nội dung bản tin (Application Layer) vẫn tuân thủ cấu trúc text-based của đề bài để tương thích với bộ Parser trong Server C.

## 5.1. Cấu trúc khung tin (Message Frame)
Mỗi message gửi qua WebSocket là một chuỗi văn bản (String) có định dạng:

### Request (Client -> Server)
```text
COMMAND [TOKEN] [DATA]
```
- **COMMAND**: Tên lệnh (Viết hoa).
- **TOKEN**: Chuỗi xác thực (để trống nếu là REGISTER/LOGIN).
- **DATA**: Dữ liệu tham số. Các trường cách nhau bởi dấu gạch đứng `|` hoặc ký tự quy định.

### Response (Server -> Client)
```text
STATUS_CODE [PAYLOAD]
```
- **STATUS_CODE**: Mã trạng thái (4 chữ số).
- **PAYLOAD**: Dữ liệu trả về (nếu có).

## 5.2. Bảng mã trạng thái (Status Codes)
| Mã | Ý nghĩa |
|---|---|
| `2000` | OK / Thành công |
| `4000` | Request không hợp lệ (Syntax error) |
| `4001` | Xung đột (Slot đã bị book, User đã tồn tại) |
| `4010` | Token thiếu |
| `4011` | Token sai hoặc hết hạn |
| `4030` | Không đủ quyền (Forbidden) |
| `4040` | Không tìm thấy (Not found) |
| `4041` | Sai mật khẩu |
| `5000` | Lỗi Server / Lỗi Database |

## 5.3. Chi tiết các lệnh (API Specs)

### 5.3.1. Xác thực (Auth)
| Hành động | Lệnh (Request) | Phản hồi (Response) |
|---|---|---|
| **Đăng ký** | `REGISTER username|password` | `2000 token` hoặc `4090` (User exist) |
| **Đăng nhập** | `LOGIN username|password` | `2000 token|role` hoặc `4041` |
| **Đăng xuất** | `LOGOUT token` | `2000` |

### 5.3.2. Giáo viên (Teacher)
| Hành động | Lệnh (Request) | Phản hồi (Response) |
|---|---|---|
| **Thêm Slot** | `ADD_SLOT token start_time|end_time` | `2000` |
| **Xóa Slot** | `DELETE_SLOT token slot_id` | `2000` |
| **Xem lịch hẹn** | `LIST_APPOINTMENTS token date` | `2000 id|time|student_names|type...` |
| **Ghi biên bản** | `ADD_MINUTES token meeting_id|base64_content` | `2000` |
| **Lấy biên bản** | `GET_MINUTES token meeting_id` | `2000 base64_content` |

### 5.3.3. Sinh viên (Student)
| Hành động | Lệnh (Request) | Phản hồi (Response) |
|---|---|---|
| **Đặt lịch đơn** | `BOOK_INDIVIDUAL token teacher_id|slot_id` | `2000 meeting_id` |
| **Đặt lịch nhóm** | `BOOK_GROUP token teacher_id|slot_id|mem1,mem2...` | `2000 meeting_id` |
| **Hủy lịch** | `CANCEL_MEETING token meeting_id` | `2000` |
| **Xem lịch tuần** | `LIST_WEEK_MEETINGS token year|week` | `2000 list_of_meetings...` |

## 5.4. Lưu ý triển khai WebSocket
- URL kết nối: `wss://<server_host>:<port>`
- Khác với TCP stream, WebSocket gửi theo Frame. Server C cần xử lý sự kiện `ON_MESSAGE` của thư viện WebSocket để lấy chuỗi payload và đưa vào hàm parser.

