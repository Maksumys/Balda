window.onload = function () {

    new Vue({
        el: '#my_uuid',
        data() {
            return {
                info: null
            };
        },
        mounted: function () {
            var req = {command: {id: 1, state: 1}};

            axios.post('/', req).then(
                response => ( this.info = response.data[ "command" ][ "uuid" ] )
            )
        } } );

};