import React, { Component } from 'react';

class ActiveUsers extends Component {
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
                                <button className='ml-auto px-4 py-2 rounded-lg bg-green-400 text-white'>Accept</button>
                                <button className='mx-4 px-4 py-2 rounded-lg bg-orange-400 text-white'>Reject</button>
                            </li>
                        ))}
                    </ul>
                </div>

            </div>
        )
    }
}

export default ActiveUsers;
