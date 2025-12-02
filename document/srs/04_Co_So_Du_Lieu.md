# 4. CƠ SỞ DỮ LIỆU (MONGODB)

Hệ thống sử dụng MongoDB để lưu trữ dữ liệu dưới dạng JSON/BSON. Dưới đây là thiết kế các Collections chính.

## 4.1. Collection `users`
Lưu trữ thông tin người dùng.

```json
{
  "_id": "ObjectId(...)",
  "username": "string (unique)",
  "password": "string (hashed)",
  "full_name": "string",
  "role": "string (TEACHER | STUDENT)",
  "created_at": "timestamp"
}
```

## 4.2. Collection `slots`
Lưu trữ các khe thời gian do giáo viên tạo ra.

```json
{
  "_id": "ObjectId(...)",
  "teacher_id": "ObjectId (ref: users)",
  "start_time": "datetime",
  "end_time": "datetime",
  "is_booked": "boolean (default: false)",
  "created_at": "timestamp"
}
```

## 4.3. Collection `meetings`
Lưu trữ thông tin cuộc họp đã được đặt và biên bản họp.
*Note: Trong thiết kế cũ, biên bản là file .txt. Trong thiết kế mới, biên bản là field `minutes` trong document này.*

```json
{
  "_id": "ObjectId(...)",
  "slot_id": "ObjectId (ref: slots)",
  "teacher_id": "ObjectId (ref: users)",
  "student_ids": [ "ObjectId (ref: users)" ], 
  "meeting_type": "string (INDIVIDUAL | GROUP)",
  "status": "string (SCHEDULED | COMPLETED | CANCELLED)",
  "minutes": "string (Base64 encoded content or Plain text)",
  "created_at": "timestamp"
}
```

## 4.4. Mối quan hệ logic
- Một **Teacher** có nhiều **Slots**.
- Một **Slot** có thể trở thành một **Meeting** khi được book.
- Một **Meeting** thuộc về một **Teacher** và một hoặc nhiều **Students**.

