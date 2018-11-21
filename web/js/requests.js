window.onload = function () {

    new Vue({
        el: '#my_uuid',
        data() {
            return {
                info: null
            };
        },
        created: function () {
            var req = {command: {id: 1, state: 1}};

            axios.post('/', req).then(
                response => ( this.info = response.data[ "command" ][ "uuid" ] )
            ).catch((error) => {
                if( error.response ){
                    console.log(error.response.data); // => the response payload
                }});
        }
    } );

    new Vue({
        el: '#run_game',

        methods: {
            runGame: function (event) {
                var req = {command: {id: 2, state: 1, uuid1: this.$refs.my_uuid, uuid2: this.$refs.my_uuid }};
                var uuid_game;

                axios.post('/', req).then(
                    response => {
                        if( ( !response.data.includes("command") ) ||
                              !response.data[ "command" ].includes( "uuid_game" ) ) {
                            console.log( "Post request error!" );
                        }
                        uuid_game = response.data[ "command" ][ "uuid_game" ]
                    }
                ).catch((error) => {
                    if( error.response ){
                        console.log(error.response.data); // => the response payload
                    }});
                // отображает таймер и клетки игры, все остальное скрываем или вообще другую
                // страницу открываем, хз
				
            }
        }
    });

	//скрывать блоки
	new Vue({
	el: '#app',
	data: {
			isNinja: true
		}
	});
};