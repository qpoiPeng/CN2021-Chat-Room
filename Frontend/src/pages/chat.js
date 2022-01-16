import React, { Component } from 'react';
import { withRouter, Link } from 'react-router-dom';
import ActiveUsers from '../components/ActiveUsers';
import Messages from './../components/messages';
import moment from 'moment';
// import LoadingScreen from 'react-loading-screen';
import axios from 'axios';
import * as Constants from '../constants';

const initialState = {
    user: '',
    friend: '',
    messages: [{from: "terrance", to: "james", text: "Hello there, James", createdDate: "4:36am"}, {from: "james", to: "terrance", text: "Hello there, Terrance", createdDate: "5.32pm"}],
    users: ["hello", "this", "is"],
    friends: ["hello"],
    requests: ["this"],
    newMsg: '',
    selectedFile: ''
}

class Chat extends Component {

    constructor(props) {
        super(props);

        this.state = {
            ...initialState
        }

        this.chatWith = this.chatWith.bind(this);
        this.refetchMessage = this.refetchMessage.bind(this);
        this.processRequest = this.processRequest.bind(this);
        this.sendRequest = this.sendRequest.bind(this);
        this.deleteFriend = this.deleteFriend.bind(this);
    }

    componentWillUnmount(){
        const param = {
            name: this.props.match.params.name
        }
        // socket.emit('leave', param);
        // * Post user leave request
        this.setState({...initialState});
    }

    componentDidMount() {

        const params = {
            user: this.props.match.params.name
        }

        // Regain information every two seconds
        // this.interval = setInterval(() => {
        //     this.getAllUsers();
        //     this.getAllFriends();
        //     this.getAllRequests();
        //     this.refetchMessage();
        // }, 2000);

        this.setState({user: params.user});

        this.getAllUsers();
        this.getAllFriends();
        this.getAllRequests();

        // Chat with friend if already have friend to chat with
        const {pathname} = this.props.location;
        const currentFriend = pathname.split("/")[3];
        if(currentFriend && currentFriend !== '') {
            // this.setState({friend: currentFriend});
            // this.chatWith(currentFriend);
            this.setState({ friend: currentFriend }, () => this.refetchMessage());
        }
    }

    getAllUsers = () => {
        const params = {
            user: this.props.match.params.name
        }
        // * Get all users list
        axios.get(`${Constants.BASEURL}/api/users`)
            .then(response => {
                console.log(response.data.userlist);
                this.setState({users: response.data.userlist.filter(function(user) {
                    return user !== params.user;
                })});
            });
    }

    getAllFriends = () => {
        // * Get user friend list
        axios.get(`${Constants.BASEURL}/friends`, {withCredentials: true})
            .then(response => {
                // console.log(response.data.friends);
                if(response.data.status === "Success") {
                    console.log(response.data.friends);
                    this.setState({friends: response.data.friends});
                }
                else
                    console.log("GET /friends failed");
            });
    }

    getAllRequests = () => {
        // * Get friend requests
        axios.get(`${Constants.BASEURL}/friends/requests`, {withCredentials: true})
            .then(response => {
                if(response.data.status === "Success") {
                    console.log(response.data['request list']);
                    this.setState({requests: response.data['request list']});
                }
                else
                    console.log("GET /friends/requests failed");
            });
    }

    scrollToBottom() {
        // selectors
        var listHeight = document.querySelector('.messages #list ul');
        var messagesList = document.querySelector('.messages #list');
        var newMessage = document.querySelector('.messages #list ul li:last-child');
        console.log(listHeight, messagesList, newMessage);
        // heights
        var messagesWrapperHeight = listHeight.clientHeight;
        var clientHeight = messagesList.clientHeight;
        var scrollTop = messagesList.scrollTop;
        var scrollHeight = messagesList.scrollHeight;
        var newMessageHeight = newMessage.offsetHeight;
        var lastMessageHeight = newMessage.previousSibling.offsetHeight;

        if (clientHeight + scrollTop + newMessageHeight + lastMessageHeight >= scrollHeight) {
            document.querySelector('#list').scrollTo(0, messagesWrapperHeight)
        }
    }

    clearForm() {
        this.setState({
            newMsg: ''
        });
    }

    inputUpdate(e) {
        const name = e.target.name;
        const value = e.target.value;
        this.setState({
            [name]: value
        });
    }

    refetchMessage() {

        var friend = this.state.friend;

        if(friend == '')
            return 0;

        var messages;

        // * Get chat history
        axios.get(`${Constants.BASEURL}/chat/${friend}`, {withCredentials: true})
            .then(response => {
                // console.log(response);
                if(response.data.status === "Success") {
                    messages = response.data.messages;

                    messages = messages.map(function(m) {

                        let unix_timestamp = m.timestamp;
                        // Create a new JavaScript Date object based on the timestamp
                        // multiplied by 1000 so that the argument is in milliseconds, not seconds.
                        var dt = new Date(unix_timestamp * 1000);

                        // Month part from the timestamp
                        var month = dt.getMonth();
                        // Date part from the timestamp
                        var date = dt.getDay();
                        // Hours part from the timestamp
                        var hours = dt.getHours();
                        // Minutes part from the timestamp
                        var minutes = "0" + dt.getMinutes();

                        // Will display time in 10:30 format
                        var formattedTime = `${hours}:${minutes.substr(-2)}`;

                        m.timestamp = formattedTime;

                        return m;
                    });

                    console.log(messages);

                    this.setState({messages: messages});

                    if (messages.length > 3) {
                        this.scrollToBottom();
                    }
                }
                else
                    console.log("GET chat history failed failed");
            });
    }

    newMessage(e) {
        e.preventDefault()

        if(this.state.newMsg === "") {
            alert("Can not send empty message");
            return 0;
        }

        // * Post create new message request
        const payload = {message: this.state.newMsg};
        axios.post(`${Constants.BASEURL}/chat/${this.state.friend}`,payload, {withCredentials: true})
            .then(response => {
                if(response.data.status === "Success") {
                    // * Get new messages from server
                    this.refetchMessage();
                }
                else {
                    alert(`Failed: ${response.data.status}`);
                }
            });
        this.clearForm();
    }

    chatWith(friend) {
        console.log(`Chat with ${friend}`);

        this.props.history.push(`/${this.state.user}/chat/${friend}`);

        this.setState({ friend: friend }, () => this.refetchMessage());
    }

    processRequest(username, action) {

        const payload = {action: action, friend_name: username};
        axios.post(`${Constants.BASEURL}/friends/requests`, payload, {withCredentials: true})
        .then(response => {
            if(response.data.status === "Success") {
                    if(action === "accept") {
                        // Add to friends
                        let friends = this.state.friends;
                        friends.push(username);
                        this.setState({friends: friends});
                    }
                    // Remove from requests
                    this.setState({requests: this.state.requests.filter(function(req) {
                        return req !== username;
                    })});
                }
                else {
                    console.log(`${response.data.status}: ${action} friend request of ${username}`);
                }
            })

    }

    sendRequest(username) {
        const payload = {action: "add", friend_name: username};
        axios.post(`${Constants.BASEURL}/friends`, payload, {withCredentials: true})
            .then(response => {
                console.log(`${response.data.status}: send friend request to ${username}`);
                if(response.data.status === "Success") {
                }
                else {
                    alert(`${response.data.status}`);
                }
            })
    }

    deleteFriend(friend_name) {
        const payload = {action: "delete", friend_name: friend_name};
        axios.post(`${Constants.BASEURL}/friends`, payload, {withCredentials: true})
            .then(response => {
                console.log(`${response.data.status}: delete friend ${friend_name}`);
                if(response.data.status === "Success") {
                    // Remove from friends
                    this.setState({friends: this.state.friends.filter(function(friend) {
                        return friend !== friend_name;
                    })});
                }
                else {
                    alert(`${response.data.status}`);
                }
            })
    }

    handleSubmit = async (event) => {
        event.preventDefault()
        const formData = new FormData();
        formData.append("selectedFile", this.state.selectedFile);

        axios.post(`${Constants.BASEURL}/sendFile/${this.state.friend}`, formData, {withCredentials: true, headers: {"Content-Type": "multipart/form-data"}})
            .then(response => {
                console.log(response.data.status);
                this.refetchMessage();
            })
            .catch(err => {
                console.log(err);
            })
    }

    handleFileSelect = (event) => {
        this.setState({selectedFile: event.target.files[0]});
    }

    render() {

        const { newMsg } = this.state;

        return (
            <div className="chatPage">

                <ActiveUsers users={this.state.users} friends={this.state.friends} requests={this.state.requests} chatWith={this.chatWith} refetchMessage={this.refetchMessage} processRequest={this.processRequest} sendRequest={this.sendRequest} deleteFriend={this.deleteFriend}/>

                <div className="messages_wrap">
                    <h1>
                        <Link to="/">
                            <i className="fas fa-chevron-circle-left"></i>
                        </Link>
                        {this.state.friend}
                    </h1>

                    <Messages messages={this.state.messages} friend={this.state.friend} requests={this.state.requests}/>

                    <div className="newMsgForm">
                        <div className="wrap">
                            <form onSubmit={(e) => this.newMessage(e)}>
                                <div className="form_wrap">
                                    <div className="form_row">
                                        <div className="form_item">
                                            <div className="form_input">
                                                <input name="newMsg" placeholder="Type your message..." autoComplete="off" value={newMsg} onChange={this.inputUpdate.bind(this)} />
                                                <span className="bottom_border"></span>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                                <div className="btnWrap">
                                    <button type="submit" className="btn">
                                        <i className="fab fa-telegram-plane"></i>
                                    </button>
                                </div>
                            </form>
                            <form onSubmit={this.handleSubmit}>
                                <input type="file" onChange={this.handleFileSelect}/>
                                <input type="submit" value="Upload File" />
                            </form>
                        </div>

                    </div>
                </div>

            </div>
        )
    }
}

export default withRouter(Chat);
