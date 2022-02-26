var PositionSearchMixin = function(myPath) {
    // This is a model mixin that manages UI state and server interaction for
    // the position/search term input and the pop-up that displays multiple results.

    'use strict';

    var myCartVars = ['position', 'positionMatches', 'geneSuggestTrack'];
    /*jshint validthis: true */

    // Server event handler
    function posMergeServerResponse(mutState, cartVar, newValue) {
        // cart vars in myCartVars all live in state[myPath] for Immutable efficiency but can arrive
        // independently from server; when we get one, update just that piece of state[myPath].
        mutState.setIn(myPath.concat(cartVar), Immutable.fromJS(newValue));
        // Turn off the loading spinner when we get result(s) from cart:
        if (cartVar === 'position' || cartVar === 'positionMatches') {
            mutState.setIn(myPath.concat('loading'), false);
        }
    }

    // UI event handlers
    function posErrorHandler(serverMessage) {
        // If the error message from the server is only that the new position/search can't be found,
        // then just display it to the user; otherwise treat it as a real error.
        if (/^Sorry, /.test(serverMessage)) {
            alert(serverMessage);
        } else {
            this.defaultServerErrorHandler(serverMessage);
        }
    }

    function lookupPosition(mutState, path, newValue) {
        // path is myPath + 'position'.
        // Tell the server to look up the new position/search term.  If there are multiple
        // matches, then the cart's position variable won't be updated unless the user
        // chooses one, and handleServerResponse will get positionMatches to show to the user.
        this.cartDo({changePosition: {newValue: newValue}}, _.bind(posErrorHandler, this));
        mutState.setIn(path, newValue);
        mutState.setIn(myPath.concat('loading'), true);
    }

    function positionMatchClick(mutState, path, matchData) {
        // path is myPath + 'positionMatch'.
        // User has clicked on a position match link; tell server the new position and
        // update the hgFindMatches cart var to highlight the user's choice.
        //#*** TODO: make track/subtrack visible!
        mutState.setIn(myPath.concat('position'), matchData.get('position'));
        mutState.setIn(myPath.concat('positionMatches'), null);
        this.cartSend({cgiVar: { position: matchData.get('position'),
                                 'hgFind.matches': matchData.get('hgFindMatches') }});
    }

    function hidePosPopup(mutState) {
        // User has clicked on the 'X' icon of position match popup box.
        // Clear positionMatches to make the popup go away.
        mutState.setIn(myPath.concat('positionMatches'), null);
    }

    // Method for use outside this mixin, e.g. for when user just changed database
    function setPosition(mutState, newPos) {
        mutState.setIn(myPath.concat('position'), newPos);
    }

    function initialize() {
        this.registerCartVarHandler(myCartVars, posMergeServerResponse);
        this.registerUiHandler(myPath.concat('position'), lookupPosition);
        this.registerUiHandler(myPath.concat('positionMatch'), positionMatchClick);
        this.registerUiHandler(myPath.concat('hidePosPopup'), hidePosPopup);
        // Install convenience methods for use outside this mixin:
        this.setPosition = setPosition;
    }

    // Mixin object with initialize
    return { initialize: initialize };
};

// Without this, jshint complains that PositionSearchMixin is not used.  Module system would help.
PositionSearchMixin = PositionSearchMixin;
