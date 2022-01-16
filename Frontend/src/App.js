import React, { Component } from 'react';
import { BrowserRouter as Router, Route, Switch } from "react-router-dom";
import classNames from 'classnames';
import './assets/scss/styles.scss';

import Chat from './pages/chat';
import Home from './pages/home'

class App extends Component {

  constructor(props) {
    super(props);
    this.state = {
      site_loaded: false
    }
  }

  componentDidMount() {
    this.setState({
      site_loaded: true
    });
  }

  render() {
    return (
      <div className={classNames({'App': true, 'site_loaded': this.state.site_loaded})}>
        <Router>
          <Switch>
            <Route path="/:name/chat" component={Chat} />
            <Route path="/" component={Home} />
          </Switch>
        </Router>
      </div>
    );
  }
}

export default App;
