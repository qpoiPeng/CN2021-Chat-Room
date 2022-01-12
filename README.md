# CN2021-Chat-Room

## API Reference

* Note: All data should be sent in JSON.

| Request Type  | URL | Content Description |
| ------------- | ------------- | --------------- |
| POST  | `$BASE_URL/register` | User register inforamtion: `{name: $username, email: $user_email, password: $user_password}` |
| POST  | `$BASE_URL/login` | User login information: `{name: $username, $user_password}` |
| GET  | `$BASE_URL/chat/$username/$friend_name` | Get single chat history |
| GET | `$BASE_URL/groupchat/$username/$group_name` | Get group chat history |
| GET | `$BASE_URL/$username/friends` | Get list of friends of username |
| POST | `$BASE_URL/$username/friends` | Add/delete friends: `{action: add/delete, friend_name: friend_name}` |
| GET | `$BASE_URL/$username/friends/requests` | Get list of friend requests (in and out) |
| POST | `$BASE_URL/$username/friends/requests` | Accept/decline friend_request: `{action: Accept/Reject, friend_name: $friend_name}` |
| GET | `$BASE_URL/$username/profile` | Get current user profile information |
| GET | `$BASE_URL/api/users` | Get all users |
| POST | `$BASE_URL/api/users` | Add/delete specific user or group: `{action: add/delete, username: $username}` |
| GET | `$BASE_URL/api/groups` | Get all groups |
| POST | `$BASE_URL/api/groups` | Add/delete specific group or group: `{action: add/delete, group_name: $group_name}` |
