# CN2021-Chat-Room

## API Reference

* Note: All data should be sent in JSON.

| Request Type  | URL | Content Description |
| ------------- | ------------- | --------------- |
| POST v | `$BASE_URL/register` | User register inforamtion: `{"name": $username, "password": $user_password}` |
| POST  v | `$BASE_URL/login` | User login information: `{"name": $username, "password":$user_password}` |
| GET  v | `$BASE_URL/chat/$friend_name` | Get single chat history |
| GET | `$BASE_URL/groupchat/$group_name` | Get group chat history |
| POST v | `$BASE_URL/chat/$friend_name` | Message: `{"message" : $message}`|
| GET v | `$BASE_URL/friends` | Get list of friends of username |
| POST v | `$BASE_URL/friends` | Add/delete friends: `{action: add/delete, friend_name: friend_name}` |
| GET v | `$BASE_URL/friends/requests` | Get list of friend requests (in and out) |
| POST v | `$BASE_URL/friends/requests` | Accept/decline friend_request: `{"action": "accept"/"reject", "friend_name": $friend_name}` |
| POST almost | `$BASE_URL/file/$filename/$friend_name` | upload a file |
| GET | `$BASE_URL/file/$fileid` | download a file |
| GET | `$BASE_URL/$username/profile` | Get current user profile information |
| GET v | `$BASE_URL/api/users` | Get all users |
| POST | `$BASE_URL/api/users` | Add/delete specific user or group: `{action: add/delete, username: $username}` |
| GET | `$BASE_URL/api/groups` | Get all groups |
| POST | `$BASE_URL/api/groups` | Add/delete specific group or group: `{action: add/delete, group_name: $group_name}` |
