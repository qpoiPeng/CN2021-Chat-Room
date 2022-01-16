import React, { Component } from 'react';

class ActiveUsers extends Component {

    // constructor(props) {
    //     super(props);
    // };

    render() {
        return (
            <div className="activeUsers">
                <h2 className="headline">
                    Active users
                </h2>
                <div id="users">
                    <ul className='py-4'>
                        {this.props.users.map((user, index) => (
                            <li key={index} className='py-4 flex'>
                                <i className={"fas fa-circle " + (this.props.friends.includes(user) ? "friend" : "non-friend")}></i>
                                <span>
                                    {user}
                                </span>

                                {this.props.friends.includes(user) ?
                                    <>
                                        <button className='ml-auto px-4 py-2 rounded-lg bg-pink-400 text-white' onClick={() => this.props.chatWith(user)}>Chat</button>
                                        <button className='ml-4 px-4 py-2 rounded-lg bg-red-400 text-white' onClick={() => this.props.deleteFriend(user)}>Delete</button>
                                    </>
                                    : this.props.requests.includes(user) ?
                                    <>
                                        <button className='ml-auto px-4 py-2 rounded-lg bg-green-400 text-white' onClick={() => this.props.processRequest(user, "accept")}>Accept</button>
                                        <button className='ml-4 px-4 py-2 rounded-lg bg-orange-400 text-white' onClick={() => this.props.processRequest(user, "reject")}>Reject</button>
                                    </>
                                    : <button className='ml-auto px-4 py-2 rounded-lg bg-blue-400 text-white' onClick={() => this.props.sendRequest(user)}>Send Request</button>
                                }
                            </li>
                        ))}
                    </ul>
                </div>

            </div>
        )
    }
}

export default ActiveUsers;
