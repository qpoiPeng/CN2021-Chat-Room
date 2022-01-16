import React, { Component } from 'react';
import * as Constants from '../constants';

class Messages extends Component {

    isImage = (filename) => {
        let splitext = filename.split(".");

        if(splitext.length <= 1)
            return false;

        return splitext[1] == "jpg" || splitext[1] == "png" || splitext[1] == "jpeg" || splitext[1] == "JPEG" || splitext[1] == "PNG";
    }

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
                                            {message.type === "message" ?
                                            <p>{message.content}</p> :
                                            this.isImage(message.content) ?
                                            <img src={Constants.BASEURL + "/server_dir/" + message.content}/> :
                                            <a href={Constants.BASEURL + "/server_dir/" + message.content}>{message.content}</a>
                                            }
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
