import React, { Component } from 'react';
import { withRouter } from 'react-router-dom';

import Register from "./register"
import Login from "./login"

const initialState = {
    tab: "login"
}

class Home extends Component {

    constructor(props) {
        super(props);
        this.changeTab = this.changeTab.bind(this);
        this.state = {
            ...initialState
        }
    }

    changeTab(tabname) {
        console.log(`Change to ${tabname} tab`);

        this.setState({
            "tab": tabname
        });
    }

    render() {

        return (
            <div>
                {this.state.tab === "login"
                    ? <Login history={this.props.history} changeTab={this.changeTab}/>
                    : <Register history={this.props.history} changeTab={this.changeTab}/>
                }
            </div>
        )
    }
}

export default withRouter(Home);

