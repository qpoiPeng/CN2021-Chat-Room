
TableName : description
columnName1, columnName2


UserList : 記錄所有的 user
id, name, password

UserInfo :
id, name, friendList (string), chatroomList


/* if type == user, content is user name, time is last read time */
DirectMessage_[user1]_[user2] :
type (user or message or file), from, content, file, time

Chatroom_[X] :
type (user or message or file), from, content, file, time

