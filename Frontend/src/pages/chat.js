import React, { Component } from 'react';
import { withRouter, Link } from 'react-router-dom';
import ActiveUsers from '../components/ActiveUsers';
import Messages from './../components/messages';
import moment from 'moment';
// import LoadingScreen from 'react-loading-screen';
import axios from 'axios';
import * as Constants from '../constants';

const initialState = {
    user: 'terrance',
    friend: 'james',
    messages: [{from: "terrance", to: "james", text: "Hello there, James", createdDate: "4:36am"}, {from: "james", to: "terrance", text: "Hello there, Terrance", createdDate: "5.32pm"}],
    users: ['qpoi', 'willy', 'LYP', 'james'],
    friends: ['qpoi', 'willy'],
    requests: [],
    newMsg: '',
}

// const postUserJoin = async (params) => {
//     const response = await axios.post("http://localhost:8081/api/join", params);
//     console.log(response.data);
// }

class Chat extends Component {

    constructor(props) {
        super(props);

        this.state = {
            ...initialState
        }

        this.chatWith = this.chatWith.bind(this);
        this.refetchMessage = this.refetchMessage.bind(this);
    }

    componentWillUnmount(){
        const param = {
            room: this.props.match.params.room
        }
        // socket.emit('leave', param);
        // * Post user leave request
        this.setState({...initialState});
    }

    componentDidMount() {

        const params = {
            name: this.props.match.params.name
        }

        // this.refetchMessage();

        // * Post user log in request
        axios.get(`${Constants.BASEURL}/api/users`)
            .then(response => {
                // console.log(response.data.userlist);
                this.setState({users: response.data.userlist});
            });

        // * Get user friend list
        axios.get(`${Constants.BASEURL}/friends`, {withCredentials: true})
            .then(response => {
                // console.log(response.data.friends);
                if(response.data.status === "Success") {
                    this.setState({friends: response.data.friends});
                }
                else
                    console.log("GET /friends failed");
            });

        // * Get friend requests
        axios.get(`${Constants.BASEURL}/friends/requests`, {withCredentials: true})
            .then(response => {
                // console.log(response);
                if(response.data.status === "Success") {
                    this.setState({requests: response.data['request list']});
                }
                else
                    console.log("GET /friends/requests failed");
            });

        // socket.on('newMessage', (message) => {
        //     var formattedTime = moment(message.createdDate).format('h:mm a');
        //     let newMsg = {
        //         text: message.text,
        //         from: message.from,
        //         room: message.room,
        //         createdDate: formattedTime
        //     }
        //     let results = scopeThis.state.messages;
        //     results.push(newMsg);
        //     scopeThis.setState({
        //         messages: results
        //     });

        //     var msgArr = scopeThis.state.messages.filter(message => message.room === this.props.match.params.room);
        //     if (msgArr.length > 3) {
        //         scopeThis.scrollToBottom();
        //     }
        // });

    }

    scrollToBottom() {
        // selectors
        var listHeight = document.querySelector('.messages #list ul');
        var messagesList = document.querySelector('.messages #list');
        var newMessage = document.querySelector('.messages #list ul li:last-child');
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

    refetchMessage(friend) {

        var messages;

        // * Get chat history
        axios.get(`${Constants.BASEURL}/chat/${friend}`, {withCredentials: true})
            .then(response => {
                // console.log(response);
                if(response.data.status === "Success") {
                    messages = response.data.messages;
                    if (messages.length > 3) {
                        this.scrollToBottom();
                    }
                    this.setState({messages: response.data.messages});
                }
                else
                    console.log("GET chat history failed failed");
            });

        // var formattedTime = moment(message.createdDate).format('h:mm a');
        // let newMsg = {
        //     text: message.text,
        //     from: message.from,
        //     room: message.room,
        //     createdDate: formattedTime
        // }

        // let results = this.state.messages;

        // results.push(newMsg);

        // this.setState({
        //     messages: results
        // });

        // var msgArr = this.state.messages.filter(message => message.room === this.props.match.params.room);

    }

    newMessage(e) {
        e.preventDefault()
        var obj = {
            'text': this.state.newMsg,
            'from': this.state.user,
            'to': this.state.friend
        };
        // * Post create new message request
        // * Get new messages from server

        // socket.emit('createMessage', obj, function (data) { });
        this.clearForm();
    }

    chatWith(friend) {
        console.log(`Chat with ${friend}`);
        this.setState({ friend: friend }, () => this.refetchMessage(this.state.friend));
    }

    render() {

        const { newMsg } = this.state;

        return (
            <div className="chatPage">

                <ActiveUsers users={this.state.users} friends={this.state.friends} requests={this.state.requests} chatWith={this.chatWith} refetchMessage={this.refetchMessage}/>

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
                        </div>

                    </div>
                </div>

            </div>
        )
    }
}

export default withRouter(Chat);
