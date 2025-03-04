import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_DATA = "data"

storage_ns = cg.esphome_ns.namespace('storage')
MediaPlayerComponent = storage_ns.class_('MediaPlayerComponent', cg.Component)

# Unique ID validation function
def validate_unique_media_player_id(full_config):
    # Track existing IDs across all configurations
    existing_ids = set()
    
    # Check for duplicates in storage configurations
    for conf in full_config.get('storage', []):
        conf_id = conf.get('id')
        if not conf_id:
            continue
        
        if conf_id in existing_ids:
            raise cv.Invalid(f"Duplicate media player ID: {conf_id}")
        
        existing_ids.add(conf_id)
    
    return full_config

# Schema for storage configuration
STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(MediaPlayerComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("media_player", lower=True),
    cv.Optional(CONF_FILES): cv.ensure_list({
        cv.Required(CONF_DATA): cv.returning_lambda,
        cv.Required(CONF_ID): cv.string,
    }),
}).extend(cv.COMPONENT_SCHEMA)

# Modify CONFIG_SCHEMA to use the validation function
CONFIG_SCHEMA = cv.Schema({
    cv.Optional('storage'): cv.All(
        cv.ensure_list(STORAGE_SCHEMA),
        validate_unique_media_player_id
    )
})

def to_code(config):
    for conf in config.get('storage', []):
        var = cg.new_Pvariable(conf[CONF_ID])
        yield cg.register_component(var, conf)
        
        cg.add(var.set_platform(conf[CONF_PLATFORM]))
        
        if CONF_FILES in conf:
            for file in conf[CONF_FILES]:
                # Process lambda for data
                data_lambda = yield cg.process_lambda(
                    file[CONF_DATA], 
                    [],  # No arguments
                    return_type=cg.std_vector(cg.uint8)
                )
                
                # Add file with processed lambda
                cg.add(var.add_file(data_lambda, file[CONF_ID]))









