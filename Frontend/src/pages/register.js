import React, { Component } from 'react';
import { withRouter } from 'react-router-dom';
import axios from 'axios';
import * as Constants from '../constants';

const initialState = {
    name: '',
    password: '',
    reenter_password: ''
}

class Register extends Component {

    constructor() {
        super();
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

    register() {
        const { name, password, reenter_password } = this.state;
        if (name && password && (password === reenter_password)) {
            // * Post HTTP register request
            const payload = { name: name, password: password };
            axios.post(`${Constants.BASEURL}/register`, payload)
                .then(response => {
                    console.log(response.data);
                    if(response.data.status === "Success") {
                        alert("Registered Successfully. Please login.");
                        this.props.changeTab("login");
                    }
                    else {
                        alert(`Register failed. Status: ${response.data.status}`);
                    }
                })
        }
    }

    render() {

        const { name, password, reenter_password } = this.state;

        return (
            <div className="registerForm">
                <div className="form_wrap">
                    <h3>Register</h3>
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
                    <div className="form_row">
                        <div className="form_item">
                            <div className="form_input">
                                <input type="text" placeholder="Reenter password" autoComplete="off" name="reenter_password" value={reenter_password} onChange={this.inputUpdate.bind(this)} />
                                <span className="bottom_border"></span>
                            </div>
                        </div>
                    </div>
                    <button onClick={() => this.props.changeTab("login")} className='pb-4 text-md underline'>Already registered? Login</button>
                    <div className="form_buttons">
                        <button onClick={() => this.register()} className="btn">
                            Register
                        </button>
                    </div>
                </div>
            </div>
        )
    }
}

export default withRouter(Register);
