import React, { Component } from 'react';
import { withRouter } from 'react-router-dom';
import axios from 'axios';
import * as Constants from '../constants';

const initialState = {
    name: '',
    password: ''
}

class Login extends Component {

    constructor(props) {
        super(props);
        this.state = {
            ...initialState
        }
    }

    clearForm() {
        this.setState({
            ...initialState
        });
    }

    inputUpdate(e) {
        const name = e.target.name.trim();
        const value = e.target.value.trim();
        this.setState({
            [name]: value
        });
    }

    login() {
        const { name, password } = this.state;
        console.log("Trying to login");
        if (name && password) {

            const payload = { name: name, password: password };
            axios.post(`${Constants.BASEURL}/login`, payload, {withCredentials: true})
                .then(response => {
                    // console.log(response);

                    if(response.data.status === "Success") {
                        this.props.history.push(`/chat/${name}`, {name: name});
                    }
                    else {
                        alert(`Login failed. Status: ${response.data.status}`);
                    }
                })
        }
    }

    render() {

        const { name, password } = this.state;

        return (
            <div className="loginForm">
                <div className="form_wrap">
                    <h3>Login</h3>
                    <div className="form_row">
                        <div className="form_item">
                            <div className="form_input">
                                <input type="text" placeholder="username" autoComplete="off" name="name" value={name} onChange={this.inputUpdate.bind(this)} />
                                <span className="bottom_border"></span>
                            </div>
                        </div>
                    </div>
                    <div className="form_row">
                        <div className="form_item">
                            <div className="form_input">
                                <input type="text" placeholder="password" autoComplete="off" name="password" value={password} onChange={this.inputUpdate.bind(this)} />
                                <span className="bottom_border"></span>
                            </div>
                        </div>
                    </div>
                    <button onClick={() => this.props.changeTab("register")} className='pb-4 text-md underline'>Not registered? Register now.</button>
                    <div className="form_buttons">
                        <button onClick={() => this.login()} className="btn">
                            Login
                        </button>
                    </div>
                </div>
            </div>
        )
    }
}

export default withRouter(Login);
