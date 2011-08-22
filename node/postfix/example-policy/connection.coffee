connection = exports

class Connection
  constructor: (@client) ->
    @current_data = ""
    @values = []
    
    @setupClient()
    
  setupClient: ->
    @client.on "data", (data) =>
      console.log "data"
      
      @process_line data
  
  process_data: ->
    console.log "Processing all of the data..."
    
    # Do something clever..
    console.log(@values)
    @client.end()
    
  process_line: (data) ->
    @current_data += data
    
    if /^\s*$/.test(data)
      console.log "End of data"
      @process_data()
    else
      m = /(\S+?)=(.*)/.exec data
		  
		  if m
		    @values[m[1]] = m[2]
		  else
			  console.log "Invalid input:", data
			  
			  @client.write "DUNNO\r\n"
			  @client.end()

exports.Connection = Connection

exports.createConnection = (client) ->
	new Connection(client)