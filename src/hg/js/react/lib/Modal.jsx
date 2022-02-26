/** @jsx React.DOM */
/* global Icon, PathUpdate, Section */
var pt = React.PropTypes;

function getScrollHeight() {
    // From https://developer.mozilla.org/en-US/docs/Web/API/Window.scrollY
    // FIXME: This belongs in a regular .js lib file.
    var supportPageOffset = window.pageXOffset !== undefined;
    var isCSS1Compat = ((document.compatMode || "") === "CSS1Compat");
    return supportPageOffset ? window.pageYOffset :
           isCSS1Compat ? document.documentElement.scrollTop : document.body.scrollTop;
}

var Modal = React.createClass({
    // A big popup pane that appears on top of the page, for use in special modes (hence "modal").
    // Don't treat this as Immutable because it wraps around children that have their own props.

    mixins: [PathUpdate],
    // update(path + 'remove') called when user clicks X icon to hide this

    propTypes: { title: pt.renderable.isRequired,  // title string or React.DOM object
               },

    componentWillUnmount: function() {
        window.scrollTo(0, this.origY);
    },

    render: function() {
        var path = this.props.path || [];
        var title = (
            <div>
              <span className='floatLeft'>
                {this.props.title}
              </span>
              <Icon type="remove" className="removeButton floatRight"
                    path={path.concat('remove')} update={this.props.update} />
              <div className='clear' />
            </div>
        );
        // Initialize Modal's position to be somewhat below the top of the current view.
        // Keep track of that position so we can maintain it in subsequent renders,
        // even if the user scrolled in the meantime.
        if (!this.top) {
            this.origY = getScrollHeight();
            this.top = this.origY + 75; // px is default unit in React style
        }

        return (
            <div style={{top: this.top}} className='absoluteModal'>
              <Section style={{margin: 0}} title={title}>
                {this.props.children}
              </Section>
            </div>
        );
    }
});

// Without this, jshint complains that Modal is not used.  Module system would help.
Modal = Modal;
