import React, { Component } from 'react';

class Messages extends Component {
    render() {
        return (
            <div className="messages">
                <div id="list">
                    <ul>
                        {this.props.messages.filter(message => (message.from === this.props.friend || message.to === this.props.friend)).map((message, index) => (
                            <li key={index}>
                                <div className='msgWrapper'>
                                    <div className={message.to === this.props.friend ? "msg align-right" : "msg"}>
                                        <h4>{message.from}</h4>
                                        <div className="body">
                                            <p>{message.content}</p>
                                        </div>
                                    </div>
                                    <span className={message.to === this.props.friend ? "createdDate text-right" : "createdDate"}>{message.timestamp}</span>
                                </div>
                            </li>
                        ))}
                    </ul>
                </div>
            </div>
        )
    }
}

export default Messages;
